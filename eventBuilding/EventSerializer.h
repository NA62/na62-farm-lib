/*
 * EventSerializer.h
 *
 *  Created on: Feb 2, 2015
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef EVENTBUILDING_EVENTSERIALIZER_H_
#define EVENTBUILDING_EVENTSERIALIZER_H_

#include <sys/types.h>
#include <atomic>

namespace na62 {

class Event;
struct EVENT_HDR;

namespace cream {
class LkrFragment;
} /* namespace cream */

class EventSerializer {
public:
	/**
	 * Generates the raw data as it should be send to the merger
	 */
	static EVENT_HDR* SerializeEvent(const Event* event);

	static void initialize();

private:
	static std::atomic<uint> InitialEventBufferSize_;
	static int TotalNumberOfDetectors_;

	static char* writeCreamData(char*& eventBuffer, uint& eventOffset,
			uint& eventBufferSize, uint& pointerTableOffset,
			cream::LkrFragment** fragments, uint numberOfFragments,
			uint sourceID);

	static char* ResizeBuffer(char* buffer, const int oldLength,
			const int newLength);
};

} /* namespace na62 */

#endif /* EVENTBUILDING_EVENTSERIALIZER_H_ */
