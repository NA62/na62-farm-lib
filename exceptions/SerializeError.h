/*
 * BrokenPacketReceivedError.h
 *
 *  Created on: Nov 15, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef SERILIZEERROR_H_
#define SERILIZEERROR_H_
#include "NA62Error.h"

namespace na62 {

class SerializeError: public na62::NA62Error {
public:
	SerializeError(std::string message) :
			na62::NA62Error(message) {
	}
};

} //namespace na62
#endif /* SERILIZEERROR_H_ */
