/*
 MEPHeader.cpp
 *
 *  Created on: Sep 21, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "MEP.h"

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

MEP::MEP(const char *data, const uint_fast16_t & dataLength,
		const DataContainer originalData) throw (BrokenPacketReceivedError,
				UnknownSourceIDFound) :
		originalData_(originalData), rawData_(reinterpret_cast<const MEP_HDR*>(data)), checkSumsVarified_(
		false) {

	fragments_ = new MEPFragment*[rawData_->eventCount];
	if (getLength() != dataLength) {
		if (getLength() > dataLength) {
			throw BrokenPacketReceivedError(
					"Incomplete MEP! Received only "
							+ std::to_string(dataLength) + " of "
							+ std::to_string(getLength()) + " bytes");
		} else {
			throw BrokenPacketReceivedError(
					"Received MEP longer than 'mep length' field! Received "
							+ std::to_string(dataLength) + " instead of "
							+ std::to_string(getLength()) + " bytes");
		}
	}

	/*
	 * Try if the sourceID is correct
	 *
	 * TODO: Do we need to check the sourceID? This is quite expensive!
	 */
	if (!SourceIDManager::checkL0SourceID(getSourceID())) {
		throw UnknownSourceIDFound(getSourceID(), getSourceSubID());
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
	delete[] fragments_;
	originalData_.free(); // Here we free the most important buffer used for polling in Receiver.cpp
}

void MEP::initializeMEPFragments(const char * data,
		const uint_fast16_t& dataLength) throw (BrokenPacketReceivedError) {
	// The first subevent starts directly after the header -> offset is 12
	uint_fast16_t offset = sizeof(MEP_HDR);

	MEPFragment* newMEPFragment;
	uint_fast32_t expectedEventNum = getFirstEventNum();

	for (uint_fast16_t i = 0; i < getNumberOfFragments(); i++) {
		/*
		 *  Throws exception if the event number LSB has an unexpected value
		 */
		newMEPFragment = new MEPFragment(this,
				(MEPFragment_HDR*) (data + offset), expectedEventNum);

//		if ((((MEPFragment_HDR*) (data + offset))->eventNumberLSB_
//				!= (expectedEventNum & 0x000000FF)) || newMEPFragment==0 || (((uint64_t)newMEPFragment->getPayload())==sizeof(MEPFragment_HDR))) {
//			LOG_INFO<< "***********MEPFragment " << newMEPFragment << ENDL;
//			LOG_INFO<< "***********MEP data " << std::hex <<(MEPFragment_HDR*) (data + offset) << std::dec<< ENDL;
//		}
		expectedEventNum++;
		fragments_[i] = newMEPFragment;
		if (newMEPFragment->getDataWithHeaderLength() + offset > dataLength) {
			throw BrokenPacketReceivedError(
					"Incomplete MEPFragment! Received only "
							+ std::to_string(dataLength) + " of "
							+ std::to_string(
									offset
											+ newMEPFragment->getDataWithHeaderLength())
							+ " bytes");
		}
		offset += newMEPFragment->getDataWithHeaderLength();
	}

	// Check if too many bytes have been transmitted
	if (offset < dataLength) {
		throw BrokenPacketReceivedError(
				"Sum of MEP events + MEP Header is smaller than expected: "
						+ std::to_string(offset) + " instead of "
						+ std::to_string(dataLength));
	}
	eventCount_ = rawData_->eventCount;
}

//bool MEP::verifyChecksums() {
//	if (checkSumsVarified_) {
//		return true;
//	}
//	checkSumsVarified_ = true;
//
//	 UDP_HDR* hdr = ( UDP_HDR*) getUDPPack();
//	if (!EthernetUtils::CheckData((char*) &hdr->ip, sizeof(iphdr))) {
//		LOG_INFO
//		<< "Packet with broken IP-checksum received" << ENDL;
//		return false;
//	}
//
//	if (!EthernetUtils::CheckUDP(hdr,
//			(const char *) (&hdr->udp) + sizeof( udphdr),
//			ntohs(hdr->udp.len) - sizeof( udphdr))) {
//		LOG_INFO
//		<< "Packet with broken UDP-checksum received"<<ENDL;
//		return false;
//	}
//	checkSumsVarified_ = true;
//	return true;
//}

} /* namespace l2 */
} /* namespace na62 */
