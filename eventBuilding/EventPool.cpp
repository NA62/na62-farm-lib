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
uint_fast32_t EventPool::poolSize_;
uint_fast32_t EventPool::largestIndexTouched_;
uint_fast32_t EventPool::mepFactor_;
uint_fast32_t EventPool::mepFactorxNodeID_;
uint_fast32_t EventPool::mepFactorxNodes_;

std::atomic<uint16_t>* EventPool::L0PacketCounter_;
std::atomic<uint16_t>* EventPool::L1PacketCounter_;

void EventPool::initialize(uint numberOfEventsToBeStored, uint numberOfNodes, uint logicalNodeID, uint mepFactor) {
	poolSize_ = numberOfEventsToBeStored;
	events_.resize(poolSize_);

    mepFactor_ = mepFactor;
    mepFactorxNodes_ = mepFactor_ * numberOfNodes;
    mepFactorxNodeID_ = mepFactor_ * logicalNodeID;

	LOG_INFO<< "Initializing EventPool with " << poolSize_
	<< " Events" << ENDL;

	/*
	 * Fill the pool with empty events.
	 */

#ifdef HAVE_TCMALLOC
        // Do it with parallel_for using tbb if tcmalloc is linked
        tbb::parallel_for(
                        tbb::blocked_range<uint_fast32_t>(0, poolSize_,
                                        poolSize_
                                                        / std::thread::hardware_concurrency()),
                        [](const tbb::blocked_range<uint_fast32_t>& r) {
                                for(size_t i=r.begin();i!=r.end(); ++i) {
                                        uint_fast32_t evID = (i - (floor(i / mepFactor_) * mepFactor_)) + (mepFactorxNodeID_ + (mepFactorxNodes_ * floor(i / mepFactor_)));
                                        events_[i] = new Event(evID);
                                }
                        });
# else
        // The standard malloc blocks-> do it singlethreaded without tcmalloc
        for (uint_fast32_t i = 0; i != poolSize_; ++i) {
        	uint_fast32_t evID = (i - floor(i / mepFactor_) * mepFactor_) + (mepFactorxNodeID_ + (mepFactorxNodes_ * floor(i / mepFactor_)));
        	events_[i] = new Event(evID);
        }
#endif


	L0PacketCounter_= new std::atomic<uint16_t>[poolSize_];
	L1PacketCounter_= new std::atomic<uint16_t>[poolSize_];
}

Event* EventPool::getEvent(uint_fast32_t eventNumber) {
    //Let's check if the eventNumber belong to the selected nodeID
    //See if event number is in a valid range of the nodeID
    int rest = (eventNumber-mepFactorxNodeID_)%mepFactorxNodes_;
    if (rest < 0 || rest >= mepFactor_){
            LOG_ERROR<<"Received Event with event number " << eventNumber
                            << " which is invalid for this nodeID"
                            << ENDL;
            return nullptr;
    }

    int myfloor = floor(eventNumber / mepFactorxNodes_);
    //int index = eventNumber - mepFactorxNodeID_ - (mepFactorxNodes_ + mepFactor_) * myfloor ;
    int index = eventNumber - mepFactorxNodeID_ + (mepFactor_- mepFactorxNodes_) * myfloor ;
    if (index >= poolSize_) {
                    LOG_ERROR<<"Received Event with event number " << eventNumber
                    << " which is higher than configured maximum number of events. Index = " << int(index) << " " << (int) (poolSize_)
                    << ENDL;
            return nullptr;
            }


    if(index>largestIndexTouched_){
            largestIndexTouched_ = index;
    }

    return events_[index];

}

void EventPool::freeEvent(Event* event) {
	event->destroy();
}

}
/* namespace na62 */
