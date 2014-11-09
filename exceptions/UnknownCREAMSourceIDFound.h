/*
 * BrokenPacketReceivedError.h
 *
 *  Created on: Nov 15, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef UNDNOWNCREAMSOURCEIDFOUND_H_
#define UNDNOWNCREAMSOURCEIDFOUND_H_
#include <stdint.h>
#include "NA62Error.h"

namespace na62 {

class UnknownCREAMSourceIDFound: public na62::NA62Error {
public:
	UnknownCREAMSourceIDFound(uint8_t crateID, uint8_t creamNum) :
			na62::NA62Error(
					"Unknown CREAM source ID: CREAM " + std::to_string((int) creamNum) + " in crate "
							+ std::to_string((int) crateID) + "\n Check the corresponding field in the Options file!") {
	}
};

} //namespace na62
#endif /* UNDNOWNCREAMSOURCEIDFOUND_H_ */
