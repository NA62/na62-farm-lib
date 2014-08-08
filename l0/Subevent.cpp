/*
 * Subevent.cpp
 *
 *  Created on: Jan 25, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "Subevent.h"

#include <new>

#include "MEPFragment.h"

namespace na62 {
namespace l0 {

Subevent::Subevent(const uint16_t expectedPacketsNum) :
		ExpectedPacketsNum(expectedPacketsNum), eventFragments(new (std::nothrow) MEPFragment*[expectedPacketsNum]), eventPartCounter(0) {
}

Subevent::~Subevent() {
//	throw NA62Error("A Subevent-Object should not be deleted! Use Subevent::destroy instead so that it can be reused by the overlaying Event!");
	destroy();
}

void Subevent::destroy() {
	for (int i = eventPartCounter - 1; i >= 0; i--) {
		delete eventFragments[i];
	}
	eventPartCounter = 0;
}
} /* namespace l0 */
} /* namespace na62 */
