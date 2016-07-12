/*
 * SmartEventSerializer.h
 *
 *  Created on: Feb 2, 2015
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef EVENTBUILDING_SMARTEVENTSERIALIZER_H_
#define EVENTBUILDING_SMARTEVENTSERIALIZER_H_

#include <sys/types.h>

#include <eventBuilding/Event.h>
#include "structs/SerialEvent.h"

namespace na62 {


struct EVENT_HDR;
struct EVENT_TRAILER;

namespace cream {
class LkrFragment;
} /* namespace cream */

class SmartEventSerializer {
public:
	/**
	 * Generates the raw data as it should be send to the merger
	 * The returned buffer must be deleted by you!
	 */
	static EVENT_HDR* SerializeEvent(const Event* event);
	static EVENT_HDR* SerializeEvent(const Event* event, l1_SerializedEvent* seriale);

	static bool compareSerializedEvent(EVENT_HDR* first_event, EVENT_HDR* second_event);

	static void initialize();

private:
	static uint InitialEventBufferSize_;
	static bool IsInitialEventBufferSizeFixed_;
	static int TotalNumberOfDetectors_;
	static bool DumpFlag_;

	static EVENT_HDR* doSerialization(const Event* event, char* eventBuffer, uint& eventBufferSize);
	static EVENT_HDR* writeHeader(const Event* event, char*& eventBuffer, uint& eventOffset, bool& isUnfinishedEOB);
	static char* writeL0Data(const Event* event, char*& eventBuffer, uint& eventOffset,
	uint& eventBufferSize, uint& pointerTableOffset, bool& isUnfinishedEOB);
	static char* writeL1Data(const Event* event, char*& eventBuffer, uint& eventOffset,
			uint& eventBufferSize, uint& pointerTableOffset, bool& isUnfinishedEOB);
	static EVENT_TRAILER* writeTrailer(const Event* event, char*& eventBuffer, uint& eventOffset, uint& eventBufferSize);

	static char* ResizeBuffer(char* buffer, const int oldLength, const int newLength);
};

} /* namespace na62 */

#endif /* EVENTBUILDING_SMARTEVENTSERIALIZER_H_ */
