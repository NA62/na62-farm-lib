/*
 * PacketHandler.cpp
 *
 *  Created on: Feb 7, 2012
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#include "PacketHandler.h"

#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/time_duration.hpp>
#include <boost/thread/pthread/thread_data.hpp>
#include <linux/pf_ring.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <zmq.h>
#include <zmq.hpp>
#include <algorithm>
#include <cstdbool>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <queue>

#include "../exceptions/UnknownCREAMSourceIDFound.h"
#include "../exceptions/UnknownSourceIDFound.h"
#include "../l0/MEP.h"
#include "../l0/MEPEvent.h"
#include "../LKr/LKRMEP.h"
#include "../messages/MessageHandler.h"
#include "../options/Options.h"
#include "../structs/Network.h"
#include "EthernetUtils.h"
#include "PFringHandler.h"
#include "ZMQHandler.h"

namespace na62 {

bool changeBurstID = false;
uint32_t nextBurstID = 0;

PacketHandler::PacketHandler() {

}

PacketHandler::~PacketHandler() {
}

void PacketHandler::connectZMQ() {
	for (int i = 0; i < Options::GetInt(OPTION_NUMBER_OF_EBS); i++) {
		EBL0sockets_.push_back(ZMQHandler::GenerateSocket(ZMQ_PUSH));
		ZMQHandler::ConnectInproc(EBL0sockets_[i],
				ZMQHandler::GetEBL0Address(i));
	}

	for (int i = 0; i < Options::GetInt(OPTION_NUMBER_OF_EBS); i++) {
		EBLKrSockets_.push_back(ZMQHandler::GenerateSocket(ZMQ_PUSH));
		ZMQHandler::ConnectInproc(EBLKrSockets_[i],
				ZMQHandler::GetEBLKrAddress(i));
	}
}

void PacketHandler::thread() {
	connectZMQ();

	DataContainer container;

	std::queue<DataContainer> dataContainers;
	register char* data; // = new char[MTU];
	struct pfring_pkthdr hdr;
	memset(&hdr, 0, sizeof(hdr));
	register int result = 0;
	int sleepMicros = 1;
	while (true) {
		result = 0;
		data = NULL;
		/*
		 * The actual  polling!
		 * Do not wait for incoming packets as this will block the ring and make sending impossible
		 */
		result = PFringHandler::GetNextPacket(&hdr, &data, 0, false,
				threadNum_);
		if (result == 1) {
			char* buff = new char[hdr.len];
			memcpy(buff, data, hdr.len);
			container = {buff, (uint16_t) hdr.len};
			dataContainers.push(std::move(container));
			sleepMicros = 1;
		} else if (dataContainers.empty()) {
			/*
			 * Use the time to send some packets
			 */
//			if (!PFringHandler::doSendPacket(threadNum_)) {
//			}
			boost::this_thread::sleep(boost::posix_time::microsec(sleepMicros));
			if (sleepMicros < 10000) {
				sleepMicros *= 2;
			}
		} else {
			if (!processPacket(dataContainers.front())) {
				dataContainers.pop();
				return; // stop running
			}
			dataContainers.pop();
		}
	}
}

bool PacketHandler::processPacket(DataContainer container) {
	std::cout << "Received packet " << container.length << " \t " << threadNum_
			<< std::endl;
	try {
		struct UDP_HDR* hdr = (struct UDP_HDR*) container.data;
		uint16_t etherType = ntohs(hdr->eth.ether_type);
		uint8_t ipProto = hdr->ip.protocol;
		uint16_t destPort = ntohs(hdr->udp.dest);

		if (etherType != ETHERTYPE_IP || ipProto != IPPROTO_UDP) {
			/*
			 * No IP or at least no UDP packet received
			 */
			if (etherType == ETHERTYPE_ARP) {
				/*
				 * Look for ARP requests asking for my IP
				 */
				struct ARP_HDR* arp = (struct ARP_HDR*) container.data;
				if (arp->targetIPAddr == PFringHandler::GetMyIP()) { // This is asking for me
					struct DataContainer responseArp =
							EthernetUtils::GenerateARPv4(
									PFringHandler::GetMyMac(),
									arp->sourceHardwAddr,
									PFringHandler::GetMyIP(), arp->sourceIPAddr,
									ARPOP_REPLY);
					PFringHandler::SendPacket(responseArp.data,
							responseArp.length);
					delete[] responseArp.data;
				}
			}

			delete[] container.data;
			return true;
		}

		/*
		 * Check IP-Header
		 */
		//				if (!EthernetUtils::CheckData((char*) &hdr->ip, sizeof(iphdr))) {
		//					mycerr( "Packet with broken IP-checksum received");
		//					delete[] container.data;
		//					continue;
		//				}
		if (ntohs(hdr->ip.tot_len) + sizeof(ether_header) != container.length) {
			/*
			 * Does not need to be equal because of ethernet padding
			 */
			if (ntohs(hdr->ip.tot_len) + sizeof(ether_header)
					> container.length) {
				mycerr(
						"Received IP-Packet with less bytes than ip.tot_len field!");
				delete[] container.data;
				return true;
			}
		}

		/*
		 * Does not need to be equal because of ethernet padding
		 */
		if (ntohs(hdr->udp.len) + sizeof(ether_header) + sizeof(iphdr)
				> container.length) {
			mycerr("Received UDP-Packet with less bytes than udp.len field!");
			delete[] container.data;
			return true;
		}

		//				/*
		//				 * Check UDP checksum
		//				 */
		//				if (!EthernetUtils::CheckUDP(hdr, (const char *) (&hdr->udp) + sizeof(struct udphdr), ntohs(hdr->udp.len) - sizeof(struct udphdr))) {
		//					mycerr( "Packet with broken UDP-checksum received" );
		//					delete[] container.data;
		//					continue;
		//				}
		const char * UDPPayload = container.data + sizeof(struct UDP_HDR);
		const uint16_t & dataLength = ntohs(hdr->udp.len)
				- sizeof(struct udphdr);

		/*
		 * UDP and IP has been checked. Now let's see what's insight the packet
		 */
		if (destPort == Options::GetInt(OPTION_L0_RECEIVER_PORT)) {
			/*
			 * L0 Data
			 * * Length is hdr->ip.tot_len-sizeof(struct udphdr) and not container.length because of ethernet padding bytes!
			 */
			l0::MEP* mep = new l0::MEP(UDPPayload, dataLength, container.data);

			for (int i = mep->getNumberOfEvents() - 1; i >= 0; i--) {
				l0::MEPEvent* event = mep->getEvent(i);

				zmq::message_t request((void*) event, event->getEventLength(),
						(zmq::free_fn*) nullptr);

				while (true) {
					try {
						EBL0sockets_[event->getEventNumber()
								% Options::GetInt(OPTION_NUMBER_OF_EBS)]->send(
								request);
						break;
					} catch (const zmq::error_t& ex) {
						if (ex.num() != EINTR) { // try again if EINTR (signal caught)
							std::cerr << ex.what() << std::endl;
							return false;
						}
					}
				}
			}

		} else if (destPort == Options::GetInt(OPTION_CREAM_RECEIVER_PORT)) {
			/*
			 * CREAM Data
			 * Length is hdr->ip.tot_len-sizeof(struct iphdr) and not container.length because of ethernet padding bytes!
			 */
			cream::LKRMEP* mep = new cream::LKRMEP(UDPPayload, dataLength,
					container.data);
			// TODO: send mep to EB
		} else if (destPort == Options::GetInt(OPTION_EOB_BROADCAST_PORT)) {
			if (dataLength != sizeof(struct EOB_FULL_FRAME) - sizeof(UDP_HDR)) {
				mycerr(
						"Unrecognizable packet received at EOB farm broadcast Port!");
				delete[] container.data;
				return true;
			}
			EOB_FULL_FRAME* pack = (struct EOB_FULL_FRAME*) container.data;
			mycout(
					"Received EOB Farm-Broadcast. Will increment BurstID now to" << pack->finishedBurstID + 1);
			changeBurstID = true;
			nextBurstID = pack->finishedBurstID + 1;
		} else {
			/*
			 * Packet with unknown UDP port received
			 */
			mycerr("Packet with unknown UDP port received: " << destPort);
			delete[] container.data;
			return true;
		}
	} catch (UnknownSourceIDFound const& e) {
		delete[] container.data;
	} catch (UnknownCREAMSourceIDFound const&e) {
		delete[] container.data;
	} catch (NA62Error const& e) {
		delete[] container.data;
	}
	return true;
}

//void PacketHandler::StartDemultiplexerThread() throw () {
//	l0::MEP* mep = NULL;
//	cream::LKRMEP* lkrMep = NULL;
//	unsigned int sleepMicros = 10;
//
//	while (true) {
//		mep = NULL;
//		lkrMep = NULL;
//
//		/*
//		 * First move L0 MEPEvents to release memory at the CREAM side
//		 * Do this as long as you can find a MEP within the input queues
//		 */
//
//		for (int thread = NumberOfHandlers - 1; thread >= 0; thread--) {
//			while (createdMEPs[thread].pop(mep)) {
//				/*
//				 * Found a non-empty queue -> send the MEPEvents to the EBs by popping until the queue is empty
//				 */
//				MEPsReceivedBySourceID_[mep->getSourceID()]++;
//				BytesReceivedBySourceID_[mep->getSourceID()] +=
//						mep->getLength();
//				for (int i = mep->getNumberOfEvents() - 1; i >= 0; i--) {
//					l0::MEPEvent* event = mep->getEvent(i);
//
//					EventsReceivedBySourceID_[mep->getSourceID()]++;
//					eventBuilders_[event->getEventNumber()
//							% Options::Instance()->NUMBER_OF_EBS]->pushMEPEvent(
//							event);
//				}
//			}
//		}
//
//		/*
//		 * If no new L0 MEP has arrived we can increment the BurstID if the broadcast has been received
//		 */
//		if (changeBurstID && mep == NULL) {
//			changeBurstID = false;
//			EventBuilder::SetNextBurstID (nextBurstID);
//		}
//		/*
//		 * Now move LKREvents
//		 * Do this only once so that we can check for L0MEPs again
//		 */
//		for (int thread = NumberOfHandlers - 1; thread >= 0; thread--) {
//			while (createdLKRMEPs[thread].pop(lkrMep)) {
//				/*
//				 * Found a non-empty queue -> send the LKREvents to the EBs
//				 */
//				MEPsReceivedBySourceID_[LKR_SOURCE_ID]++;
//				BytesReceivedBySourceID_[LKR_SOURCE_ID] += lkrMep->getLength();
//				for (int i = lkrMep->getNumberOfEvents() - 1; i >= 0; i--) {
//					cream::LKREvent* event = lkrMep->getEvent(i);
//					EventsReceivedBySourceID_[LKR_SOURCE_ID]++;
//					eventBuilders_[event->getEventNumber()
//							% Options::Instance()->NUMBER_OF_EBS]->pushLKREvent(
//							event);
//				}
//			}
//		}
//
//		if (mep == NULL && lkrMep == NULL) {
//			/*
//			 * No empty queue found -> fight the greenhouse effect and save energy!
//			 */
//			if (Options::DO_SHUTDOWN) {
//				std::cout << "Stopping StorageHandler Demultiplexer"
//						<< std::endl;
//				return;
//			}
//			boost::this_thread::sleep(boost::posix_time::microsec(sleepMicros));
//			if (sleepMicros < 10000) {
//				sleepMicros *= 2;
//			}
//		} else {
//			sleepMicros = 1000;
//		}
//	}
//}

} /* namespace na62 */
