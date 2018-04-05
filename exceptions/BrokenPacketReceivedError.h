/*
 * BrokenPacketReceivedError.h
 *
 *  Created on: Nov 15, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef FREECONTAINER_H_
#define FREECONTAINER_H_

#include "NA62Error.h"

namespace na62 {

class FreeContainer: public na62::NA62Error {
public:
	FreeContainer(const std::string& message) :
			na62::NA62Error(message) {
	}
};

} //namespace na62
#endif /* FREECONTAINER_H_ */
