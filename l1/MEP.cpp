/*
 * MEP.cpp
 *
 *  Created on: Mar 2, 2016
 *      Author: giovanna
 */


#include "MEP.h"

#include <boost/lexical_cast.hpp>
#include <string>

#include "../exceptions/CommonExceptions.h"
#include "../exceptions/BrokenPacketReceivedError.h"
#include "../exceptions/UnknownSourceIDFound.h"

namespace na62 {
namespace l1 {

MEP::MEP(const char * data, const uint16_t& dataLength,
                DataContainer etherFrame) :
                dataContainer_(etherFrame), sourceID_(0xff) {

        /*
         * There is no special MEP header! A MEP just consists of several MEPFragments and we have to
         * find out how many of those are written into this packet.
         */
	eventNum_ = 0 ;

	initializeMEPFragments(data, dataLength);
}

MEP::~MEP() {
        if (eventNum_ != 0) {
                /*
                 * TODO: Just for testing. Should be deleted later to boost performance!
                 */
#ifdef USE_ERS
        	ers::error(BadInternalDataFlow(ERS_HERE, "Deleting non-empty MEP!!!"));
#else
                throw NA62Error("Deleting non-empty MEP!!!");
#endif
        }
        dataContainer_.free();
}

void MEP::initializeMEPFragments(const char * data, const uint16_t& dataLength) {
	if(dataLength == 0) {
#ifdef USE_ERS
		throw CorruptedMEP(ERS_HERE, "Received EMPTY UDP packet!");
#else
		throw BrokenPacketReceivedError("Received EMPTY UDP packet!");
#endif
	}
	if(dataLength < sizeof(L1_EVENT_RAW_HDR)) {
#ifdef USE_ERS
		std::ostringstream s;
		s << "Received UDP packet with size " << dataLength << " less data than a single L1 header!";
		throw CorruptedMEP(ERS_HERE, s.str());
#else
		throw BrokenPacketReceivedError("Received UDP packet with less data than a single L1 header!");
#endif
	}
	// Get source ID from first fragment and check its validity
	const L1_EVENT_RAW_HDR* hdr = (const L1_EVENT_RAW_HDR*)(data);

	if (!SourceIDManager::checkL1SourceID(hdr->sourceID)) {
#ifdef USE_ERS
		throw UnknownSourceID(ERS_HERE, hdr->sourceID, hdr->sourceSubID);
#else
		throw UnknownSourceIDFound(hdr->sourceID, hdr->sourceSubID);
#endif
	}

	sourceID_ = hdr->sourceID;

	uint16_t offset = 0;

	MEPFragment* newEvent = nullptr;

	while (offset < dataLength) {

		newEvent = new MEPFragment(this, (const L1_EVENT_RAW_HDR*)(data + offset));

		if (newEvent->getEventLength()>9500) {
			std::ostringstream s;
			s << "Negative event length in L1 MEP" << newEvent->getEventLength();
#ifdef USE_ERS
			throw CorruptedMEP(ERS_HERE, s.str());
#else
			throw BrokenPacketReceivedError(s.str());
#endif
		}

		if (newEvent->getEventLength() + offset > dataLength) {
			std::ostringstream s;
			s << "Incomplete L1 MEP Fragment. Received only " << (uint) dataLength << " instead of " <<
					(uint) (offset + newEvent->getEventLength()) << " bytes.";
#ifdef USE_ERS
			throw CorruptedMEP(ERS_HERE, s.str());
#else
			throw BrokenPacketReceivedError(s.str());
#endif
		}
		offset += newEvent->getEventLength();
		events.push_back(std::move(newEvent));
	}

	eventNum_ = events.size();
}

} /* namespace l1 */
} /* namespace na62 */
