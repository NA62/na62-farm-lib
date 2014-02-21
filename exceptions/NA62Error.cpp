/*
 * NA62Error.cpp
 *
 *  Created on: Nov 15, 2011
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#include "NA62Error.h"
#include "../messages/MessageHandler.h"
namespace na62 {

NA62Error::NA62Error(const std::string& message) :
		std::runtime_error(message) {
	mycerr( message );
}

} /* namespace na62 */
