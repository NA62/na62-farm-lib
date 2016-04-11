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

#include "../exceptions/CommonExceptions.h"
#include "../exceptions/BrokenPacketReceivedError.h"
#include "../exceptions/UnknownSourceIDFound.h"
#include "../options/Options.h"
#include "MEPFragment.h"

namespace na62 {
namespace l0 {

MEP::MEP(const char *data, const uint_fast16_t & dataLength,
		const DataContainer originalData) :
		originalData_(originalData), rawData_(reinterpret_cast<const MEP_HDR*>(data)), checkSumsVarified_(
		false) {

	if (dataLength < sizeof(MEP_HDR)) {
#ifdef USE_ERS
		std::ostringstream s;
		s << "Incomplete MEP! Size " << dataLength << " smaller than MEP_HDR!";
		throw CorruptedMEP(ERS_HERE, s.str());
#else
		throw BrokenPacketReceivedError(
							"type = BadEv : Incomplete MEP! Size " + std::to_string(dataLength) + " smaller than MEP_HDR!"
									+ std::to_string(dataLength) + " of "
									+ std::to_string(getLength()) + " bytes");
#endif
	}
	fragments_ = new MEPFragment*[rawData_->eventCount];
	if (getLength() != dataLength) {
		if (getLength() > dataLength) {
#ifdef USE_ERS
			std::ostringstream s;
			s<< "BadEv : Incomplete MEP! Received only " << dataLength << " of " << getLength() << " bytes";
			throw CorruptedMEP(ERS_HERE, s.str());
#else
			throw BrokenPacketReceivedError(
					"type = BadEv : Incomplete MEP! Received only "
							+ std::to_string(dataLength) + " of "
							+ std::to_string(getLength()) + " bytes");
#endif
		} else {
#ifdef USE_ERS
			std::ostringstream s;
			s << "Received MEP longer than 'mep length' field! Received "
			  << dataLength << " instead of " << getLength() <<  " bytes";
			throw CorruptedMEP(ERS_HERE, s.str());
#else
			throw BrokenPacketReceivedError(
					"type = BadEv : Received MEP longer than 'mep length' field! Received "
							+ std::to_string(dataLength) + " instead of "
							+ std::to_string(getLength()) + " bytes");
#endif
		}

	}

	/*
	 * Try if the sourceID is correct
	 *
	 * TODO: Do we need to check the sourceID? This is quite expensive!
	 */
	if (!SourceIDManager::checkL0SourceID(getSourceID())) {
#ifdef USE_ERS
		throw UnknownSourceID(ERS_HERE, getSourceID(), getSourceSubID());
#else
		throw UnknownSourceIDFound(getSourceID(), getSourceSubID());
#endif
	}
	initializeMEPFragments(data, dataLength);
}

MEP::~MEP() {
	if (eventCount_ > 0) {
		/*
		 * TODO: Just for testing. Should be deleted later to boost performance!
		 */
		//throw NA62Error("Deleting non-empty MEP!!!");
#ifdef USE_ERS
		ers::error(Message("Deleting non-empty MEP!!!"));
#endif;
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

		expectedEventNum++;
		fragments_[i] = newMEPFragment;
		if (newMEPFragment->getDataWithHeaderLength() + offset > dataLength) {
#ifdef USE_ERS
			std::ostringstream s;
			s << "Incomplete MEPFragment! Received only " << dataLength << " of "
			  << offset + newMEPFragment->getDataWithHeaderLength() << " bytes";
			throw CorruptedMEP(ERS_HERE, s.str());
#else
			throw BrokenPacketReceivedError(
					"type = BadEv : Incomplete MEPFragment! Received only "
							+ std::to_string(dataLength) + " of "
							+ std::to_string(
									offset
											+ newMEPFragment->getDataWithHeaderLength())
							+ " bytes");
#endif
			}
		offset += newMEPFragment->getDataWithHeaderLength();
	}

	// Check if too many bytes have been transmitted
	if (offset < dataLength) {
#ifdef USE_ERS
		std::ostringstream s;
		s << "Sum of MEP events + MEP Header is smaller than expected: " << offset << " instead of " << dataLength;
		throw CorruptedMEP(ERS_HERE, s.str());
#else
		throw BrokenPacketReceivedError(
				"type = BadEv : Sum of MEP events + MEP Header is smaller than expected: "
						+ std::to_string(offset) + " instead of "
						+ std::to_string(dataLength));
#endif

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
