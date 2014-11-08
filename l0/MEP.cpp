/*
 MEPHeader.cpp
 *
 *  Created on: Sep 21, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "MEP.h"

#ifdef USE_GLOG
#include <glog/logging.h>
#endif
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <iostream>
#include <new>
#include <string>

#include "../exceptions/BrokenPacketReceivedError.h"
#include "../exceptions/UnknownSourceIDFound.h"
#include "../options/Options.h"
#include "MEPFragment.h"

namespace na62 {
namespace l0 {

MEP::MEP(const char *data, const uint16_t & dataLength,
		const char *originalData) throw (BrokenPacketReceivedError,
				UnknownSourceIDFound) :
		etherFrame_(originalData), rawData((struct MEP_HDR*) (data)), checkSumsVarified_(
				false) {

	fragments = new MEPFragment*[rawData->eventCount];
	if (getLength() != dataLength) {
		if (getLength() > dataLength) {
			throw BrokenPacketReceivedError(
					"Incomplete MEP! Received only "
							+ std::to_string(dataLength)
							+ " of "
							+ std::to_string(getLength())
							+ " bytes");
		} else {
			throw BrokenPacketReceivedError(
					"Received MEP longer than 'mep length' field! Received "
							+ std::to_string(dataLength)
							+ " instead of "
							+ std::to_string(getLength())
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
	initializeMEPFragments(data, dataLength);
}

MEP::~MEP() {
	if (eventCount_ > 0) {
		/*
		 * TODO: Just for testing. Should be deleted later to boost performance!
		 */
		throw NA62Error("Deleting non-empty MEP!!!");
	}
	delete[] fragments;
	delete[] etherFrame_; // Here we free the most important buffer used for polling in Receiver.cpp
}

void MEP::initializeMEPFragments(const char * data, const uint16_t& dataLength)
		throw (BrokenPacketReceivedError) {
	// The first subevent starts directly after the header -> offset is 12
	uint16_t offset = sizeof(MEP_HDR);

	MEPFragment* newMEPFragment;
	uint32_t expectedEventNum = getFirstEventNum();

	for (uint16_t i = 0; i < getNumberOfEvents(); i++) {
		/*
		 *  Throws exception if the event number LSB has an unexpected value
		 */
		newMEPFragment = new  MEPFragment(this,
				(MEPFragment_HDR*) (data + offset), expectedEventNum);

		expectedEventNum++;
		fragments[i] = newMEPFragment;
		if (newMEPFragment->getDataWithHeaderLength() + offset > dataLength) {
			throw BrokenPacketReceivedError(
					"Incomplete MEPFragment! Received only "
							+ std::to_string(dataLength)
							+ " of "
							+ std::to_string(
									offset + newMEPFragment->getDataWithHeaderLength())
							+ " bytes");
		}
		offset += newMEPFragment->getDataWithHeaderLength();
	}

	// Check if too many bytes have been transmitted
	if (offset < dataLength) {
		throw BrokenPacketReceivedError(
				"Sum of MEP events + MEP Header is smaller than expected: "
						+ std::to_string(offset)
						+ " instead of "
						+ std::to_string(dataLength));
	}
	eventCount_ = rawData->eventCount;
}

//bool MEP::verifyChecksums() {
//	if (checkSumsVarified_) {
//		return true;
//	}
//	checkSumsVarified_ = true;
//
//	struct UDP_HDR* hdr = (struct UDP_HDR*) getUDPPack();
//	if (!EthernetUtils::CheckData((char*) &hdr->ip, sizeof(iphdr))) {
//#ifdef USE_GLOG
//		LOG(INFO)
//#else
//		std::cerr
//#endif
//		<< "Packet with broken IP-checksum received";
//		return false;
//	}
//
//	if (!EthernetUtils::CheckUDP(hdr,
//			(const char *) (&hdr->udp) + sizeof(struct udphdr),
//			ntohs(hdr->udp.len) - sizeof(struct udphdr))) {
//#ifdef USE_GLOG
//		LOG(INFO)
//#else
//		std::cerr
//#endif
//		<< "Packet with broken UDP-checksum received";
//		return false;
//	}
//	checkSumsVarified_ = true;
//	return true;
//}

} /* namespace l2 */
} /* namespace na62 */
