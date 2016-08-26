/*
 * SmartEventSerializer.h
 *
 *  Created on: Jul 19, 2016
 *      Author: Marco Boretto
 */

#ifndef EVENTBUILDING_SMARTEVENTSERIALIZER_H_
#define EVENTBUILDING_SMARTEVENTSERIALIZER_H_

#include <sys/types.h>

#include "eventBuilding/Event.h"
#include "structs/SerialEvent.h"

namespace na62 {


struct EVENT_HDR;
struct EVENT_TRAILER;

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
	static uint inline getInitialEventBufferSize(){
		return  InitialEventBufferSize_;
	}

private:
	static uint InitialEventBufferSize_;
	static int TotalNumberOfDetectors_;
	static bool DumpFlag_;

	static EVENT_HDR* doSerialization(const Event* event, char* eventBuffer, uint& eventBufferSize, bool& isInitialEventBufferSizeFixed);
	static EVENT_HDR* writeHeader(const Event* event, char*& eventBuffer, uint& eventOffset, bool& isUnfinishedEOB);
	static char* writeL0Data(const Event* event, char*& eventBuffer, uint& eventOffset,
	uint& eventBufferSize, uint& pointerTableOffset, bool& isUnfinishedEOB, bool& isInitialEventBufferSizeFixed);
	static char* writeL1Data(const Event* event, char*& eventBuffer, uint& eventOffset,
			uint& eventBufferSize, uint& pointerTableOffset, bool& isUnfinishedEOB, bool& isInitialEventBufferSizeFixed);
	static EVENT_TRAILER* writeTrailer(const Event* event, char*& eventBuffer, uint& eventOffset, uint& eventBufferSize, bool& isInitialEventBufferSizeFixed);

	static char* ResizeBuffer(char* buffer, const int oldLength, const int newLength, bool& isInitialEventBufferSizeFixed);
};

} /* namespace na62 */

#endif /* EVENTBUILDING_SMARTEVENTSERIALIZER_H_ */
