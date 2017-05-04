/*
 * NA62Error.cpp
 *
 *  Created on: Nov 15, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "NA62Error.h"

#include <string>
#include "../options/Logging.h"
namespace na62 {

NA62Error::NA62Error(const std::string& message) :
		std::runtime_error(message) {
	LOG_INFO(message);
}

}
/* namespace na62 */
