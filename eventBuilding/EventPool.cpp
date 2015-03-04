/*
 * EventPool.cpp
 *
 *  Created on: Jul 1, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "EventPool.h"

#include <tbb/tbb.h>
#include <thread>
#include <iostream>

#include "../options/Logging.h"

#include "Event.h"

namespace na62 {

std::vector<Event*> EventPool::events_;
uint32_t EventPool::numberOfEventsStored_;
uint32_t EventPool::largestEventNumberTouched_;

void EventPool::initialize(uint numberOfEventsToBeStored) {
	numberOfEventsStored_ = numberOfEventsToBeStored;
	events_.resize(numberOfEventsStored_);

	LOG_INFO<< "Initializing EventPool with " << numberOfEventsStored_
	<< " Events" << ENDL;

	/*
	 * Fill the pool with empty events.
	 */
#ifdef HAVE_TCMALLOC
	// Do it with parallel_for using tbb if tcmalloc is linked
	tbb::parallel_for(
			tbb::blocked_range<uint32_t>(0, numberOfEventsStored_,
					numberOfEventsStored_
							/ std::thread::hardware_concurrency()),
			[](const tbb::blocked_range<uint32_t>& r) {
				for(size_t eventNumber=r.begin();eventNumber!=r.end(); ++eventNumber) {
					events_[eventNumber] = new Event(eventNumber);
				}
			});
# else
	// The standard malloc blocks-> do it singlethreaded without tcmalloc
	for (uint32_t eventNumber = 0; eventNumber != numberOfEventsStored_;
			++eventNumber) {
		events_[eventNumber] = new Event(eventNumber);
	}
#endif
}

Event* EventPool::getEvent(uint32_t eventNumber) {
	if (eventNumber >= numberOfEventsStored_) {
		LOG_ERROR<<"Received Event with event number " << eventNumber
		<< " which is higher than configured maximum number of events"
		<< ENDL;

		return nullptr;
	}
	if(eventNumber>largestEventNumberTouched_){
		largestEventNumberTouched_ = eventNumber;
	}
	return events_[eventNumber];
}

void EventPool::freeEvent(Event* event) {
	event->destroy();
}

}
/* namespace na62 */
