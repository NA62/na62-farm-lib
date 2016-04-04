/*
 * MEP.cpp
 *
 *  Created on: Mar 2, 2016
 *      Author: giovanna
 */


#include "MEP.h"

#include <boost/lexical_cast.hpp>
#include <string>

#include "../exceptions/BrokenPacketReceivedError.h"
#include "../exceptions/UnknownSourceIDFound.h"

namespace na62 {
namespace l1 {

MEP::MEP(const char * data, const uint16_t& dataLength,
                DataContainer etherFrame) :
                dataContainer_(etherFrame), sourceID_(0) {

        /*
         * There is no special MEP header! A MEP just consists of several MEPFragments and we have to
         * find out how many of those are written into this packet.
         */
        initializeMEPFragments(data, dataLength);
}

MEP::~MEP() {
        if (eventNum_ != 0) {
                /*
                 * TODO: Just for testing. Should be deleted later to boost performance!
                 */
                throw NA62Error("Deleting non-empty MEP!!!");
        }
        dataContainer_.free();
}

void MEP::initializeMEPFragments(const char * data, const uint16_t& dataLength) {

		eventNum_ = 0 ;
		events.clear();
        uint16_t offset = 0;

        MEPFragment* newEvent = nullptr;

        if(dataLength == 0) {
        	LOG_ERROR << "Received EMPTY UDP packet!";
        	return;
        }
        if(dataLength < sizeof(L1_EVENT_RAW_HDR)) {
               	LOG_ERROR << "Received UDP packet with less data than a single L1 header!";
               	return;
        }

        while (offset < dataLength) {
                newEvent = new MEPFragment(this, (const L1_EVENT_RAW_HDR*)(data + offset));

                if (newEvent->getEventLength() + offset > dataLength) {
                        throw BrokenPacketReceivedError(
                                        "Incomplete MEPFragment! Received only  "
                                                        + boost::lexical_cast<std::string>(dataLength)
                                                        + " instead of "
                                                        + boost::lexical_cast<std::string>(
                                                                        offset + newEvent->getEventLength())
                                                        + " bytes");
                }
                offset += newEvent->getEventLength();
                events.push_back(std::move(newEvent));
        }
        if (newEvent)
        	sourceID_ = newEvent->getSourceID();
        eventNum_ = events.size();
}



} /* namespace l1 */
} /* namespace na62 */
