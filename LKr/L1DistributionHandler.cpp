/*
 * L1DistributionHandler.cpp
 *
 *  Created on: Mar 3, 2012
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#include "L1DistributionHandler.h"

#include <arpa/inet.h>
#include <bits/atomic_base.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>
#include <boost/asio/ip/multicast.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/time_duration.hpp>
#include <boost/thread/pthread/thread_data.hpp>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <new>
#include <string>
#include <glog/logging.h>

#include "../eventBuilding/Event.h"
#include "../eventBuilding/SourceIDManager.h"
#include "../options/Options.h"
#include "../socket/PFringHandler.h"
#include "../structs/Network.h"

namespace na62 {
namespace cream {
ThreadsafeQueue<struct TRIGGER_RAW_HDR*>* L1DistributionHandler::multicastMRPQueues;
ThreadsafeQueue<unicastTriggerAndCrateCREAMIDs_type>* L1DistributionHandler::unicastMRPWithIPsQueues;

struct cream::MRP_FRAME_HDR* L1DistributionHandler::CREAM_MulticastRequestHdr;
struct cream::MRP_FRAME_HDR* L1DistributionHandler::CREAM_UnicastRequestHdr;

uint64_t L1DistributionHandler::L1TriggersSent = 0;
uint64_t L1DistributionHandler::L1MRPsSent = 0;

bool L1DistributionHandler::paused = false;

uint MAX_TRIGGERS_PER_L1MRP = 0;

std::vector<DataContainer> L1DistributionHandler::MRPQueues;
boost::mutex L1DistributionHandler::sendMutex_;

static boost::asio::ip::udp::endpoint multicast_endpoint_;
boost::asio::io_service io_service;
static boost::asio::ip::udp::socket multicast_socket_(io_service,
		multicast_endpoint_.protocol());

struct cream::TRIGGER_RAW_HDR* generateTriggerHDR(const Event * event,
bool zSuppressed) {
	struct cream::TRIGGER_RAW_HDR* triggerHDR =
			new struct cream::TRIGGER_RAW_HDR();
#ifdef __USE_BIG_ENDIAN_FOR_MRP
	triggerHDR->timestamp = htonl(event->getTimestamp());
	triggerHDR->fineTime = event->getFinetime();
	triggerHDR->requestZeroSuppressed = zSuppressed;
	triggerHDR->triggerTypeWord = htons(event->getTriggerTypeWord());
	triggerHDR->eventNumber = htonl(event->getEventNumber()) >> 8;
#else
	triggerHDR->timestamp = event->getTimestamp();
	triggerHDR->fineTime = event->getFinetime();
	triggerHDR->requestZeroSuppressed = zSuppressed;
	triggerHDR->triggerTypeWord = event->getTriggerTypeWord();
	triggerHDR->eventNumber = event->getEventNumber();
#endif
	return triggerHDR;
}

void L1DistributionHandler::Async_RequestLKRDataMulticast(
		const uint16_t threadNum, Event * event, bool zSuppressed) {
	struct cream::TRIGGER_RAW_HDR* triggerHDR = generateTriggerHDR(event,
			zSuppressed);
	while (!multicastMRPQueues[threadNum].push(triggerHDR)) {
		LOG(ERROR)<< "L1DistributionHandler input queue overrun!";
		usleep(1000);
	}
}

void L1DistributionHandler::Async_RequestLKRDataUnicast(
		const uint16_t threadNum, const Event *event, bool zSuppressed,
		const std::vector<uint16_t> crateCREAMIDs) {
	struct cream::TRIGGER_RAW_HDR* triggerHDR = generateTriggerHDR(event,
			zSuppressed);
	auto pair = std::make_pair(triggerHDR, crateCREAMIDs);
	while (!unicastMRPWithIPsQueues[threadNum].push(pair)) {
		LOG(ERROR)<<"L1DistributionHandler input queue overrun!";
		usleep(1000);
	}
}

void L1DistributionHandler::Initialize() {
	MAX_TRIGGERS_PER_L1MRP = Options::GetInt(OPTION_MAX_TRIGGERS_PER_L1MRP);
	void* rawData = operator new[](
			Options::GetInt(OPTION_NUMBER_OF_EBS)
					* sizeof(ThreadsafeQueue<struct TRIGGER_RAW_HDR*> ));
	multicastMRPQueues =
			static_cast<ThreadsafeQueue<struct TRIGGER_RAW_HDR*>*>(rawData);

	for (int i = Options::GetInt(OPTION_NUMBER_OF_EBS) - 1; i != -1; i--) {
		new (&multicastMRPQueues[i]) ThreadsafeQueue<struct TRIGGER_RAW_HDR*>(
				100000);
	}
	rawData =
			operator new[](
					Options::GetInt(OPTION_NUMBER_OF_EBS)
							* sizeof(ThreadsafeQueue<
									unicastTriggerAndCrateCREAMIDs_type> ));
	unicastMRPWithIPsQueues = static_cast<ThreadsafeQueue<
			unicastTriggerAndCrateCREAMIDs_type>*>(rawData);

	for (int i = Options::GetInt(OPTION_NUMBER_OF_EBS) - 1; i >= 0; i--) {
		new (&unicastMRPWithIPsQueues[i]) ThreadsafeQueue<
				unicastTriggerAndCrateCREAMIDs_type>(100000);
	}

	char * CREAM_MulticastRequestBuff = new char[MTU];
	char * CREAM_UnicastRequestBuff = new char[MTU];

	CREAM_MulticastRequestHdr =
			(struct cream::MRP_FRAME_HDR*) (CREAM_MulticastRequestBuff);
	CREAM_UnicastRequestHdr =
			(struct cream::MRP_FRAME_HDR*) (CREAM_UnicastRequestBuff);

	const uint32_t multicastGroup = inet_addr(
			Options::GetString(OPTION_CREAM_MULTICAST_GROUP).data());
	uint16_t sPort = Options::GetInt(OPTION_CREAM_RECEIVER_PORT);
	uint16_t dPort = Options::GetInt(OPTION_CREAM_MULTICAST_PORT);
	EthernetUtils::GenerateUDP(CREAM_MulticastRequestBuff,
			EthernetUtils::GenerateMulticastMac(multicastGroup), multicastGroup,
			sPort, dPort);
	/*
	 * TODO: The router MAC has to be set here:
	 */
	EthernetUtils::GenerateUDP(CREAM_UnicastRequestBuff,
			EthernetUtils::StringToMAC("00:11:22:33:44:55"),
			0/*Will be set later*/, sPort, dPort);

	CREAM_MulticastRequestHdr->MRP_HDR.ipAddress = PFringHandler::GetMyIP();
	CREAM_MulticastRequestHdr->MRP_HDR.reserved = 0;

	CREAM_UnicastRequestHdr->MRP_HDR.ipAddress = PFringHandler::GetMyIP();
	CREAM_UnicastRequestHdr->MRP_HDR.reserved = 0;

//	EthernetUtils::GenerateUDP(CREAM_RequestBuff, EthernetUtils::StringToMAC("00:15:17:b2:26:fa"), "10.0.4.3", sPort, dPort);

	/*
	 * Sending Multicast via Vanilla Kernel sockets
	 */
// set ttl
	const boost::asio::ip::multicast::hops option(128);
	multicast_socket_.set_option(option);

	multicast_endpoint_ = boost::asio::ip::udp::endpoint(
			boost::asio::ip::address::from_string(
					Options::GetString(OPTION_CREAM_MULTICAST_GROUP)),
			Options::GetInt(OPTION_CREAM_MULTICAST_PORT));

}

void L1DistributionHandler::thread() {
	Initialize();

	/*
	 * Round robin counter
	 */
	int threadNum = 0;

	/*
	 * Counts how often threadNum run around. After each time we should send even if we did not collect MAX_TRIGGERS_PER_L1MRP yet
	 */
	int EBIterations = 0;

	/*
	 * We need all MRPs for each CREAM  but the multicastMRPQueues stores it in the opposite order (one MRP, several CREAMs).
	 * Therefore we will fill the following map and later  produce unicast IP packets with it
	 */
	std::vector<struct TRIGGER_RAW_HDR*> unicastRequestsByCrateCREAMID[SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT];

	std::vector<struct TRIGGER_RAW_HDR*> multicastRequests;
	multicastRequests.reserve(MAX_TRIGGERS_PER_L1MRP);
	const unsigned int sleepMicros = Options::GetInt(
	OPTION_MIN_USEC_BETWEEN_L1_REQUESTS);
	while (true) {
		ThreadsafeQueue<struct TRIGGER_RAW_HDR*>* queue =
				&multicastMRPQueues[threadNum
						% Options::GetInt(OPTION_NUMBER_OF_EBS)];

		const int NUMBER_OF_EBS = Options::GetInt(OPTION_NUMBER_OF_EBS);
		while (multicastRequests.size() < MAX_TRIGGERS_PER_L1MRP) {
			struct TRIGGER_RAW_HDR* hdr;
			if (queue->pop(hdr)) {
				multicastRequests.push_back(hdr);
			} else {
				if (++EBIterations == NUMBER_OF_EBS) {
					/*
					 * Now we should send the MRP although we didn't summon MAX_TRIGGERS_PER_L1MRP as all queues seem to be (almost) empty
					 */
					EBIterations = 0;
					break;
				}
				/*
				 * Check the next queue
				 */
				queue = &multicastMRPQueues[++threadNum % NUMBER_OF_EBS];
			}
		}

		/*
		 * Now send all unicast requests
		 */
		unicastTriggerAndCrateCREAMIDs_type unicastMRPWithCrateCREAMID;
		for (int thread = NUMBER_OF_EBS - 1; thread != -1; thread--) { // every EB thread
			while (unicastMRPWithIPsQueues[thread].pop(
					unicastMRPWithCrateCREAMID)) { // every entry in the EBs queue containing MRP+list of IPs
				for (uint32_t localCREAMID : unicastMRPWithCrateCREAMID.second) { // every IP
					/*
					 * Add the MRP to unicastRequestsByIP with IP as key
					 */
					unicastRequestsByCrateCREAMID[localCREAMID].push_back(
							unicastMRPWithCrateCREAMID.first);
				}
			}
		}

		if (multicastRequests.size() > 0) {
			Async_SendMRP(CREAM_MulticastRequestHdr, multicastRequests);
		} else {
			bool didSendUnicastMRPs = false;
			for (int i =
					SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT
							- 1; i != -1; i--) {
				std::vector<struct TRIGGER_RAW_HDR*> triggers =
						unicastRequestsByCrateCREAMID[i];
				if (triggers.size() > 0) {
					Async_SendMRP(CREAM_UnicastRequestHdr, triggers);
					didSendUnicastMRPs = true;
				}
			}

			if (!didSendUnicastMRPs) {
				/*
				 * In the last iteration all queues have been empty -> sleep a while
				 *
				 * The rate of MRPs should be about 100kHz/MAX_TRIGGERS_PER_L1MRP which is about 1kHz
				 * So within 1ms we will gather enough triggers for one MRP
				 */
				boost::this_thread::sleep(
						boost::posix_time::microsec(sleepMicros));
			}
		}
	}
	std::cerr << "Unexpected exit of L1DistributionHandler thread" << std::endl;
	exit(1);
}

bool L1DistributionHandler::DoSendMRP(const uint16_t threadNum) {
	if (sendMutex_.try_lock() && !MRPQueues.empty()) {
		DataContainer container = MRPQueues.back();
		MRPQueues.pop_back();

		PFringHandler::SendPacketConcurrently(threadNum, container.data,
				container.length);

		sendMutex_.unlock();
		return true;
	}
	return false;
}

/*
 * The pf_ring version for the 10G link
 */
void L1DistributionHandler::Async_SendMRP(struct cream::MRP_FRAME_HDR* dataHDR,
		std::vector<struct TRIGGER_RAW_HDR*>& triggers) {
	uint16_t offset = sizeof(struct cream::MRP_FRAME_HDR);

	uint numberOfTriggers = 0;
	while (triggers.size() != 0 && numberOfTriggers != MAX_TRIGGERS_PER_L1MRP) {
		struct TRIGGER_RAW_HDR* trigger = triggers.back();
		triggers.pop_back();

		memcpy(reinterpret_cast<char*>(dataHDR) + offset, trigger,
				sizeof(struct cream::TRIGGER_RAW_HDR));
		offset += sizeof(struct cream::TRIGGER_RAW_HDR);

		delete trigger;
		numberOfTriggers++;
	}

	dataHDR->SetNumberOfTriggers(numberOfTriggers);
	dataHDR->udp.ip.check = 0;
	dataHDR->udp.ip.check = EthernetUtils::GenerateChecksum(
			(const char*) (&dataHDR->udp.ip), sizeof(struct iphdr));
	dataHDR->udp.udp.check = EthernetUtils::GenerateUDPChecksum(&dataHDR->udp,
			dataHDR->MRP_HDR.MRPLength);

	char* buff = new char[offset];
	memcpy(buff, dataHDR, offset);
	MRPQueues.push_back( { buff, offset });

	L1TriggersSent += numberOfTriggers;
	L1MRPsSent++;

}
}
/* namespace cream */
} /* namespace na62 */
