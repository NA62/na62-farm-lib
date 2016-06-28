/*
 * MEPFragment.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "MEPFragment.h"

#include <string>

#include "../options/Logging.h"
#include "../exceptions/BrokenPacketReceivedError.h"
#include "MEP.h"  // forward declaration
//#include "../options/Logging.h"

using namespace na62;
namespace na62 {
namespace l1 {

MEPFragment::MEPFragment(MEP* mep, const L1_EVENT_RAW_HDR * data) :
		mep_(mep), rawData_(data) {


	dataLength_ = rawData_->numberOf4BWords * 4;
	//dataLength_ = 140;
}


MEPFragment::~MEPFragment() {
	if (mep_->deleteEvent()) {
		delete mep_;
	}
}


} /* namespace l1 */
} /* namespace na62 */
