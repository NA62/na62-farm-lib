/*
 * EventPool.h
 *
 *  Created on: Jul 1, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef EVENTPOOL_H_
#define EVENTPOOL_H_

#include <sys/types.h>
#include <cstdint>
#include <vector>

namespace na62 {
class Event;

class EventPool {
private:
	static std::vector<Event*> events_;
	static uint32_t numberOfEventsStored_;
public:
	static void Initialize(uint numberOfEventsToBeStored);
	static Event* GetEvent(uint32_t eventNumber);
	static void FreeEvent(Event* event);
};

} /* namespace na62 */

#endif /* EVENTPOOL_H_ */
