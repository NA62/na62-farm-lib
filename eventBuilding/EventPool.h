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
#include <atomic>

namespace na62 {
class Event;
class EventPool {
private:
	static std::vector<Event*> events_;
	static uint_fast32_t poolSize_;
    static uint_fast32_t mepFactor_;
     static uint_fast32_t mepFactorxNodeID_;
     static uint_fast32_t mepFactorxNodes_;


	static std::atomic<uint16_t>* L0PacketCounter_;
	static std::atomic<uint16_t>* L1PacketCounter_;
	/*
	 * Largest eventnumber that was passed to GetEvent
	 */
	static uint_fast32_t largestIndexTouched_;
public:
	static void initialize(uint numberOfEventsToBeStored, uint numberOfNodes=1, uint logicalNodeID=0, uint mepFactor=0);
	static Event* getEvent(uint_fast32_t eventNumber);
    static Event* getEventByIndex(uint_fast32_t index){
            if (index>=poolSize_) return nullptr;
            return events_[index];
    }

    static void freeEvent(Event* event);

	static uint_fast32_t getLargestTouchedEventnumberIndex(){
		return largestIndexTouched_;
	}
	static uint_fast32_t getPoolSize(){
			return poolSize_;
	}
	static std::atomic<uint16_t>* getL0PacketCounter(){
		return L0PacketCounter_;
	}
	static std::atomic<uint16_t>* getL1PacketCounter(){
		return L1PacketCounter_;
	}
};

} /* namespace na62 */

#endif /* EVENTPOOL_H_ */
