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
uint_fast32_t EventPool::numberOfEventsStored_;
uint_fast32_t EventPool::largestEventIndexTouched_;
uint_fast32_t EventPool::eventNumberOffset_ = 0;
uint_fast32_t EventPool::eventNumberPeriod_ = 1;

std::atomic<uint16_t>* EventPool::L0PacketCounter_;
std::atomic<uint16_t>* EventPool::CREAMPacketCounter_;

void EventPool::initialize(uint numberOfEventsToBeStored, uint eventNumberOffset, uint eventNumberPeriod) {
	eventNumberOffset_ = eventNumberOffset;
	eventNumberPeriod_ = eventNumberPeriod;
	numberOfEventsStored_ = (numberOfEventsToBeStored - eventNumberOffset_) / eventNumberPeriod_ + 1;
	events_.resize(numberOfEventsStored_);

	LOG_INFO<< "Initializing EventPool with " << numberOfEventsStored_
	<< " Events" << ENDL;

	/*
	 * Fill the pool with empty events.
	 */
#ifdef HAVE_TCMALLOC
	// Do it with parallel_for using tbb if tcmalloc is linked
	tbb::parallel_for(
			tbb::blocked_range<uint_fast32_t>(0, numberOfEventsStored_,
					numberOfEventsStored_
							/ std::thread::hardware_concurrency()),
			[](const tbb::blocked_range<uint_fast32_t>& r) {
				for(size_t eventIndex=r.begin();eventIndex!=r.end(); ++eventIndex) {
					events_[eventIndex] = new Event(eventIndex * eventNumberPeriod_ + eventNumberOffset_);
				}
			});
# else
	// The standard malloc blocks-> do it singlethreaded without tcmalloc
	for (uint_fast32_t eventIndex = 0; eventIndex != numberOfEventsStored_;
			++eventIndex) {
		events_[eventIndex] = new Event(eventIndex * eventNumberPeriod_ + eventNumberOffset_);
	}
#endif
	L0PacketCounter_= new std::atomic<uint16_t>[numberOfEventsStored_];
	CREAMPacketCounter_= new std::atomic<uint16_t>[numberOfEventsStored_];
}

Event* EventPool::getEvent(uint_fast32_t eventNumber) {
	if ((eventNumber - eventNumberOffset_)%eventNumberPeriod_ != 0) {
		LOG_ERROR<<"Received Event with event number " << eventNumber
		<< " which should not be received by this PC"
		<< ENDL;

		return nullptr;
	}
	uint_fast32_t eventIndex = (eventNumber - eventNumberOffset_)/eventNumberPeriod_;
	if (eventIndex >= numberOfEventsStored_) {
		LOG_ERROR<<"Received Event with event number(index) " << eventNumber << "(" << eventIndex << ")"
		<< " which is outside the ensemble of expected events"
		<< ENDL;

		return nullptr;
	}
	if(eventIndex>largestEventIndexTouched_){
		largestEventIndexTouched_ = eventIndex;
	}
	return events_[eventIndex];
}

Event* EventPool::getEventByIdx(uint_fast32_t eventIndex) {
	return events_[eventIndex];
}

void EventPool::freeEvent(Event* event) {
	event->destroy();
}

}
/* namespace na62 */
