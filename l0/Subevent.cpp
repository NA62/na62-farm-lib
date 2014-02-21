/*
 * Subevent.cpp
 *
 *  Created on: Jan 25, 2012
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#include "Subevent.h"

#include <new>

#include "MEPEvent.h"

namespace na62 {
namespace l0 {

Subevent::Subevent(const uint16_t expectedPacketsNum) :
		ExpectedPacketsNum(expectedPacketsNum), eventParts(new (std::nothrow) MEPEvent*[expectedPacketsNum]), eventPartCounter(0) {
}

Subevent::~Subevent() {
//	throw NA62Error("A Subevent-Object should not be deleted! Use Subevent::destroy instead so that it can be reused by the overlaying Event!");
	destroy();
}

void Subevent::destroy() {
	for (int i = eventPartCounter - 1; i >= 0; i--) {
		delete eventParts[i];
	}
	eventPartCounter = 0;
}
} /* namespace l0 */
} /* namespace na62 */
