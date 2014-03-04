/*
 * StorageHandler.h
 *
 *  Created on: Mar 4, 2014
 *      Author: root
 */

#ifndef STORAGEHANDLER_H_
#define STORAGEHANDLER_H_

#include <cstdint>

namespace na62 {
class Event;
} /* namespace na62 */

namespace na62 {

class StorageHandler {
public:
	StorageHandler();
	virtual ~StorageHandler();

	static int Async_SendEvent(const uint16_t& thredNum, Event* event);
};

} /* namespace na62 */

#endif /* STORAGEHANDLER_H_ */
