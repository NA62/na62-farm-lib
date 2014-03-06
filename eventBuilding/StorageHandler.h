/*
 * StorageHandler.h
 *
 *  Created on: Mar 4, 2014
 *      Author: root
 */

#ifndef STORAGEHANDLER_H_
#define STORAGEHANDLER_H_

#include <zmq.hpp>
#include <cstdint>
#include <vector>

namespace na62 {
class Event;
} /* namespace na62 */

namespace na62 {

class StorageHandler {
public:
	static void Initialize();

	static int SendEvent(const uint16_t& thredNum, Event* event);

private:
	/*
	 * One Socket for every EventBuilder
	 */
	static std::vector<zmq::socket_t*> MergerSockets_;
};

} /* namespace na62 */

#endif /* STORAGEHANDLER_H_ */
