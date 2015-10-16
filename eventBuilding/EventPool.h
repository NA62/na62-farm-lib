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
	static uint_fast32_t numberOfEventsStored_;

	static std::atomic<uint16_t>* L0PacketCounter_;
	static std::atomic<uint16_t>* CREAMPacketCounter_;
	/*
	 * Largest eventnumber that was passed to GetEvent
	 */
	static uint_fast32_t largestEventIndexTouched_;
	static uint_fast32_t eventNumberOffset_;
	static uint_fast32_t eventNumberPeriod_;
public:
	static void initialize(uint numberOfEventsToBeStored, uint eventNumberOffset, uint eventNumberPeriod);
	static Event* getEvent(uint_fast32_t eventNumber);
	static Event* getEventByIdx(uint_fast32_t eventIndex);
	static void freeEvent(Event* event);

	static uint_fast32_t getLargestEventIndexTouched(){
		return largestEventIndexTouched_;
	}
	static void resetLargestEventIndexTouched(){
		largestEventIndexTouched_ = 0;
	}
	static uint_fast32_t getnumberOfEventsStored(){
			return numberOfEventsStored_;
	}
	static uint_fast32_t getEventNumber(uint_fast32_t eventIndex){
		return eventIndex * eventNumberPeriod_ + eventNumberOffset_;
	}
	static uint_fast32_t getEventIndex(uint_fast32_t eventNumber){
		return (eventNumber - eventNumberOffset_)/eventNumberPeriod_;
	}
	static std::atomic<uint16_t>* getL0PacketCounter(){
		return L0PacketCounter_;
	}
	static std::atomic<uint16_t>* getL0PacketCounterByEvNum(uint_fast32_t eventNumber){
		return &L0PacketCounter_[(eventNumber - eventNumberOffset_)/eventNumberPeriod_];
	}
	static std::atomic<uint16_t>* getCREAMPacketCounter(){
		return CREAMPacketCounter_;
	}
	static std::atomic<uint16_t>* getCREAMPacketCounterByEvNum(uint_fast32_t eventNumber){
		return &CREAMPacketCounter_[(eventNumber - eventNumberOffset_)/eventNumberPeriod_];
	}
};

} /* namespace na62 */

#endif /* EVENTPOOL_H_ */
