/*
 * Subevent.cpp
 *
 *  Created on: Mar 2, 2016
 *      Author: giovanna
 */



#include "Subevent.h"

#include <new>

#include "MEPFragment.h"

namespace na62 {
namespace l1 {

Subevent::Subevent(const uint_fast16_t expectedPacketsNum, const uint_fast8_t sourceID) :
		expectedPacketsNum(expectedPacketsNum), sourceID(sourceID), eventFragments(
				new (std::nothrow) MEPFragment*[expectedPacketsNum]), fragmentCounter(
				0) {
}

Subevent::~Subevent() {
	throw NA62Error("A L1Subevent-Object should not be deleted! Use L1Subevent::destroy instead so that it can be reused by the overlaying Event!");
	destroy();
	delete[] eventFragments;
}

void Subevent::destroy() {
	for (uint_fast16_t i = 0; i != fragmentCounter; i++) {
		delete eventFragments[i];
		eventFragments[i] = nullptr;
	}
	fragmentCounter = 0;
}

}
}
