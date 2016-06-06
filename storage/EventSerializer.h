/*
 * EventSerializer.h
 *
 *  Created on: Feb 2, 2015
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef EVENTBUILDING_EVENTSERIALIZER_H_
#define EVENTBUILDING_EVENTSERIALIZER_H_

#include <sys/types.h>

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
	 * The returned buffer must be deleted by you!
	 */
	static EVENT_HDR* SerializeEvent(const Event* event);

	static void initialize();

private:
	static uint InitialEventBufferSize_;
	static int TotalNumberOfDetectors_;
	static bool DumpFlag_;
	static char* writeL0Data(const Event* event, char*& eventBuffer, uint& eventOffset,
	uint& eventBufferSize, uint& pointerTableOffset);
	static char* writeL1Data(const Event* event, char*& eventBuffer, uint& eventOffset,
			uint& eventBufferSize, uint& pointerTableOffset);

	static char* ResizeBuffer(char* buffer, const int oldLength,
			const int newLength);
};

} /* namespace na62 */

#endif /* EVENTBUILDING_EVENTSERIALIZER_H_ */
