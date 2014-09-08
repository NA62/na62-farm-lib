/*
 * EventPool.cpp
 *
 *  Created on: Jul 1, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "EventPool.h"

#include <tbb/tbb.h>
#ifdef USE_GLOG
#include <glog/logging.h>
#endif
#include <thread>
#include <iostream>

#include "Event.h"

namespace na62 {

std::vector<Event*> EventPool::events_;
uint32_t EventPool::numberOfEventsStored_;

void EventPool::Initialize(uint numberOfEventsToBeStored) {
	numberOfEventsStored_ = numberOfEventsToBeStored;
	events_.resize(numberOfEventsStored_);

	std::cout << "Initializing EventPool with " << numberOfEventsStored_
			<< " Events" << std::endl;

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

Event* EventPool::GetEvent(uint32_t eventNumber) {
	if (eventNumber >= numberOfEventsStored_) {
#ifdef USE_GLOG
		LOG(ERROR)
#else
		std::cerr
#endif
		<<"Received Event with event number " << eventNumber
		<< " which is higher than configured maximum number of events"
#ifndef USE_GLOG
		<< std::endl
#endif
		;

		return nullptr;
	}
	return events_[eventNumber];
}

void EventPool::FreeEvent(Event* event) {
	event->destroy();
}

}
/* namespace na62 */
