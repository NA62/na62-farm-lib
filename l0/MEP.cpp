/*
 MEPHeader.cpp
 *
 *  Created on: Sep 21, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "MEP.h"

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <iostream>
#include <new>
#include <string>

#include "../exceptions/BrokenPacketReceivedError.h"
#include "../exceptions/UnknownSourceIDFound.h"
#include "../options/Options.h"
#include "MEPEvent.h"

namespace na62 {
namespace l0 {

MEP::MEP(const char *data, const uint16_t & dataLength,
		const char *originalData) throw (BrokenPacketReceivedError,
				UnknownSourceIDFound) :
		etherFrame_(originalData), rawData((struct MEP_RAW_HDR*) (data)), checkSumsVarified_(
				false) {

	events = new (std::nothrow) MEPEvent*[rawData->eventCount];
	if (getLength() != dataLength) {
		if (getLength() > dataLength) {
			throw BrokenPacketReceivedError(
					"Incomplete MEP! Received only "
							+ boost::lexical_cast<std::string>(dataLength)
							+ " of "
							+ boost::lexical_cast<std::string>(getLength())
							+ " bytes");
		} else {
			throw BrokenPacketReceivedError(
					"Received MEP longer than 'mep length' field! Received "
							+ boost::lexical_cast<std::string>(dataLength)
							+ " instead of "
							+ boost::lexical_cast<std::string>(getLength())
							+ " bytes");
		}
	}

	/*
	 * Try if the sourceID is correct
	 *
	 * TODO: Do we need to check the sourceID? This is quite expensive!
	 */
	if (!SourceIDManager::CheckL0SourceID(getSourceID())) {
		throw UnknownSourceIDFound(getSourceID());
	}
	initializeMEPEvents(data, dataLength);
}

MEP::~MEP() {
	if (this->getNumberOfEvents() > 0) {
		/*
		 * TODO: Just for testing. Should be deleted later to boost performance!
		 */
		throw NA62Error("Deleting non-empty MEP!!!");
	}
	delete[] events;
	delete[] etherFrame_; // Here we free the most important buffer used for polling in Receiver.cpp
}

void MEP::initializeMEPEvents(const char * data, const uint16_t& dataLength)
		throw (BrokenPacketReceivedError) {
	// The first subevent starts directly after the header -> offset is 12
	uint16_t offset = sizeof(MEP_RAW_HDR);

	MEPEvent* newMepEvent;
	uint32_t expectedEventNum = getFirstEventNum();

	for (uint16_t i = 0; i < getNumberOfEvents(); i++) {
		/*
		 *  Throws exception if the event number LSB has an unexpected value
		 */
		newMepEvent = new (std::nothrow) MEPEvent(this, (MEPEVENT_HDR*)data + offset,
				expectedEventNum);

		expectedEventNum++;
		events[i] = newMepEvent;
		if (newMepEvent->getDataLength() + offset > dataLength) {
			throw BrokenPacketReceivedError(
					"Incomplete MEPEvent! Received only "
							+ boost::lexical_cast<std::string>(dataLength)
							+ " of "
							+ boost::lexical_cast<std::string>(
									offset + newMepEvent->getDataLength())
							+ " bytes");
		}
		offset += newMepEvent->getDataLength();
	}

	// Check if too many bytes have been transmitted
	if (offset < dataLength) {
		throw BrokenPacketReceivedError(
				"Sum of MEP events + MEP Header is smaller than expected: "
						+ boost::lexical_cast<std::string>(offset)
						+ " instead of "
						+ boost::lexical_cast<std::string>(dataLength));
	}
}

//bool MEP::verifyChecksums() {
//	if (checkSumsVarified_) {
//		return true;
//	}
//	checkSumsVarified_ = true;
//
//	struct UDP_HDR* hdr = (struct UDP_HDR*) getUDPPack();
//	if (!EthernetUtils::CheckData((char*) &hdr->ip, sizeof(iphdr))) {
//		LOG(ERROR)<< "Packet with broken IP-checksum received";
//		return false;
//	}
//
//	if (!EthernetUtils::CheckUDP(hdr,
//			(const char *) (&hdr->udp) + sizeof(struct udphdr),
//			ntohs(hdr->udp.len) - sizeof(struct udphdr))) {
//		LOG(ERROR)<< "Packet with broken UDP-checksum received";
//		return false;
//	}
//	checkSumsVarified_ = true;
//	return true;
//}

} /* namespace l2 */
} /* namespace na62 */
