/*
 * NA62Error.cpp
 *
 *  Created on: Nov 15, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "NA62Error.h"

#include <string>
#ifdef USE_GLOG
#include <glog/logging.h>
#endif
namespace na62 {

NA62Error::NA62Error(const std::string& message) :
		std::runtime_error(message) {
#ifdef USE_GLOG
	LOG(INFO) << message;
#endif
}

}
/* namespace na62 */
