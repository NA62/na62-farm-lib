/*
 * MEPEvent.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#include "LKREvent.h"

#include "../exceptions/BrokenPacketReceivedError.h"
#include "../exceptions/UnknownCREAMSourceIDFound.h"
#include "../options/Options.h"
#include "LKRMEP.h"  // forward declaration

using namespace na62;
namespace na62 {
namespace cream {

LKREvent::LKREvent(LKRMEP* mep, const char *data, const uint16_t& dataLength) throw (NA62Error) :
		mep_(mep), rawData((const struct LKR_EVENT_RAW_HDR*) data), data_(data) {
	if (rawData->LKRsourceID != 0x24) {
		throw BrokenPacketReceivedError("LKR Event with a non 0x24 source field received!");
	}
	if (!Options::CheckCREAMID(getCrateID(), getCREAMID())) {
		throw UnknownCREAMSourceIDFound(getCrateID(), getCREAMID());
	}
}

LKREvent::~LKREvent() {
	if (mep_->deleteEvent()) {
		delete mep_;
	}
}

} /* namespace cream */
} /* namespace na62 */
