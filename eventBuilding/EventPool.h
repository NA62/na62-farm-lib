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

	/*
	 * Largest eventnumber that was passed to GetEvent
	 */
	static uint32_t largestEventNumberTouched_;
public:
	static void initialize(uint numberOfEventsToBeStored);
	static Event* getEvent(uint32_t eventNumber);
	static void freeEvent(Event* event);

	static uint32_t getLargestTouchedEventnumber(){
		return largestEventNumberTouched_;
	}
};

} /* namespace na62 */

#endif /* EVENTPOOL_H_ */
