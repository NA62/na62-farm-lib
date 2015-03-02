/*
 * MEPFragment.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "LkrFragment.h"

#include "../eventBuilding/SourceIDManager.h"
#include "../exceptions/BrokenPacketReceivedError.h"
#include "../exceptions/UnknownCREAMSourceIDFound.h"

using namespace na62;
namespace na62 {
namespace cream {

LkrFragment::LkrFragment(const char *data, const uint16_t& dataLength,
		const char* etherFrame) throw (NA62Error) :
		rawData((const struct LKR_EVENT_RAW_HDR*) data), data_(data), etherFrame_(
				etherFrame) {
	if (rawData->LKRsourceID != 0x24) {
		throw BrokenPacketReceivedError(
				"LKR Event with a non 0x24 source field received!");
	}
	if (!SourceIDManager::checkCREAMID(getCrateID(), getCREAMID())) {
		throw UnknownCREAMSourceIDFound(getCrateID(), getCREAMID());
	}
}

LkrFragment::~LkrFragment() {
	delete[] etherFrame_;
}

} /* namespace cream */
} /* namespace na62 */
