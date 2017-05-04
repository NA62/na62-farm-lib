/*
 * BrokenPacketReceivedError.h
 *
 *  Created on: Nov 15, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef UNDNOWNSOURCEIDFOUND_H_
#define UNDNOWNSOURCEIDFOUND_H_
#include <stdint.h>
#include <string>

#include "NA62Error.h"

namespace na62 {

class UnknownSourceIDFound: public na62::NA62Error {
public:
	UnknownSourceIDFound(uint_fast8_t sourceID, uint subSourceID) :
			na62::NA62Error(
					"Unknown source ID: " + std::to_string((int) sourceID) + " board " + std::to_string((int)subSourceID)
							+ "\n Check the corresponding field in the Options file!") {
	}
};

} //namespace na62
#endif /* UNDNOWNSOURCEIDFOUND_H_ */
