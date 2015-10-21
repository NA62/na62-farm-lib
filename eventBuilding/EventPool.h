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

	static std::atomic<uint16_t>* L0PacketCounter_;
	static std::atomic<uint16_t>* CREAMPacketCounter_;
	/*
	 * Largest eventnumber that was passed to GetEvent
	 */
	static uint_fast32_t largestEventNumberTouched_;
public:
	static void initialize(uint numberOfEventsToBeStored);
	static Event* getEvent(uint_fast32_t eventNumber);
	static void freeEvent(Event* event);

	static uint_fast32_t getLargestTouchedEventnumber(){
		return largestEventNumberTouched_;
	}
	static uint_fast32_t getPoolSize(){
			return poolSize_;
	}
	static std::atomic<uint16_t>* getL0PacketCounter(){
		return L0PacketCounter_;
	}
	static std::atomic<uint16_t>* getCREAMPacketCounter(){
		return CREAMPacketCounter_;
	}
};

} /* namespace na62 */

#endif /* EVENTPOOL_H_ */
