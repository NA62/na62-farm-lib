/*
 * MEPFragment.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "MEPFragment.h"

#include <boost/lexical_cast.hpp>
#include <string>

#include "../exceptions/BrokenPacketReceivedError.h"
#include "MEP.h"  // forward declaration

using namespace na62;
namespace na62 {
namespace l0 {

MEPFragment::MEPFragment(MEP* mep, const MEPFragment_HDR *data,
		uint32_t& expectedEventNum) :
		mep_(mep), rawData((const struct MEPFragment_HDR*) data), eventNumber_(
				expectedEventNum) {
	/*
	 * Cite from NA62-11-02:
	 * Event number LSB: the least significant 16 bits of the event number, as defined inside the
	 * transmitter by the number of L0 triggers received since the start of the burst; the most
	 * significant 8 bits are obtained from the MEP header. For the first event in the MEP, this field
	 * will match the lower 16 bits of the first word in the MEP header; since all sub-systems in global
	 * mode must respond to all L0 triggers, for each following event in the MEP, this number should
	 * increase by one, possibly wrapping around to zero (in which case the upper 8 bits of the event
	 * number are those in the MEP header incremented by one).
	 */
	if (rawData->eventNumberLSB_ != (expectedEventNum & 0x000000FF)) {
		throw BrokenPacketReceivedError(
				"MEPFragment with bad event number LSB received: received "
						+ boost::lexical_cast<std::string>(
								(int) rawData->eventNumberLSB_)
						+ " but expected LSB is "
						+ boost::lexical_cast<std::string>(
								expectedEventNum & 0xFF000000));
	}
}

MEPFragment::~MEPFragment() {
	if (mep_->deleteEvent()) {
		delete mep_;
	}
}

/*
 * The sourceID in the header of this MEP event
 */
const uint8_t MEPFragment::getSourceID() const {
	return mep_->getSourceID();
}

/*
 * The sourceSubID in the header of this MEP event
 */
const uint8_t MEPFragment::getSourceSubID() const {
	return mep_->getSourceSubID();
}

/*
 * The internally used number corresponding to the sourceID of this MEP event
 */
const uint8_t MEPFragment::getSourceIDNum() const {
	return mep_->getSourceIDNum();
}

} /* namespace l0 */
} /* namespace na62 */
