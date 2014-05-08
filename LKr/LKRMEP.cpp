/*
 LKRMEP.cpp
 *
 *  Created on: Sep 21, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "LKRMEP.h"

#include <boost/lexical_cast.hpp>
#include <string>

#include "../exceptions/BrokenPacketReceivedError.h"

namespace na62 {
namespace cream {

LKRMEP::LKRMEP(const char * data, const uint16_t& dataLength, const char* originalData) throw (BrokenPacketReceivedError, UnknownCREAMSourceIDFound) :
		orignialData(originalData), length_(dataLength) {

	/*
	 * There is now special LKRMEP header! A MEP just contains of several LKREvents and we have to find out how many of those are written into this packet.
	 */
	initializeLKREvents(data, dataLength);
}

LKRMEP::~LKRMEP() {
	if (eventNum > 0) {
		/*
		 * TODO: Just for testing. Should be deleted later to boost performance!
		 */
		throw NA62Error("Deleting non-empty LKRMEP!!!");
	}

	delete[] orignialData; // Here we free the most important buffer used for polling in Receiver.cpp
}

void LKRMEP::initializeLKREvents(const char * data, const uint16_t& dataLength) throw (UnknownCREAMSourceIDFound, BrokenPacketReceivedError) {
	uint16_t offset = 0;

	LKREvent* newEvent;

	while (offset < dataLength) {
		newEvent = new LKREvent(this, data + offset, dataLength);
		events.push_back(newEvent);

		if (newEvent->getEventLength() + offset > dataLength) {
			throw BrokenPacketReceivedError(
					"Incomplete LKREvent! Received only " + boost::lexical_cast<std::string>(dataLength) + " instead of "
							+ boost::lexical_cast<std::string>(offset + newEvent->getEventLength()) + " bytes");
		}
		offset += newEvent->getEventLength();
	}
	eventNum = events.size();
}

} /* namespace cream */
} /* namespace na62 */
