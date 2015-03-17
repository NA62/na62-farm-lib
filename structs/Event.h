/*
 * Event.hpp
 *
 *  Created on: Mar 7, 2014
 \*      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef EVENT_HPP_
#define EVENT_HPP_

#include <cstdint>

/*
 * Special triggers
 */
#define TRIGGER_L0_SOB 0x22
#define TRIGGER_L0_EOB 0x23

#define TRIGGER_L1_BYPASS 0xFF

#define TRIGGER_L2_BYPASS 0xFF

namespace na62 {
/**
 * Pointer to the payload of the given sourceID within an event
 */
struct EVENT_DATA_PTR {
	uint32_t offset :24; // Number of 4B-Words from the beginning of the Event
	uint8_t sourceID;
};

/*
 * UDP/IP complete header
 *
 */
struct EVENT_HDR {
	uint_fast32_t eventNum :24;
	uint_fast8_t format;

	uint_fast32_t length; // number of 4B-words
	uint_fast32_t burstID;
	uint_fast32_t timestamp;

	uint_fast32_t triggerWord :24;
	uint_fast8_t reserved1;

	uint_fast8_t fineTime;
	uint_fast8_t numberOfDetectors;
	uint_fast16_t reserved2;

	uint_fast32_t processingID;

	uint_fast32_t SOBtimestamp;

	/**
	 * Returns the pointers from the pointer table to the data of all source IDs. Use it as following:
	 ' EVENT_DATA_PTR* sourceIdAndOffsets = event->getDataPointer();
	 ' for(int sourceNum=0; sourceNum!=event->numberOfDetectors; sourceNum++){
	 ' 	EVENT_DATA_PTR* sourceIdAndOffset = sourceIdAndOffsets[sourceNum];
	 ' }
	 */
	EVENT_DATA_PTR* getDataPointer() {
		return (EVENT_DATA_PTR*) (((char*) this) + sizeof(EVENT_HDR));
	}

	uint_fast8_t getL0TriggerTypeWord() {
		return triggerWord & 0xFF;
	}

	uint_fast8_t getL1TriggerTypeWord() {
		return triggerWord >> 8 & 0xFF;
	}

	uint_fast8_t getL2TriggerTypeWord() {
		return triggerWord >> 16 & 0xFF;
	}

}__attribute__ ((__packed__));

/*
 * Header for each L0 data block coming from one Tel62 board. This allows to recognize the data block end even if the data coming
 * from the electronics is broken
 */
struct L0_BLOCK_HDR {
	uint_fast16_t dataBlockSize;
	uint_fast8_t sourceSubID;
	uint_fast8_t reserved;
}__attribute__ ((__packed__));

struct EVENT_TRAILER {
	uint_fast32_t eventNum :24;
	uint_fast8_t reserved;
}__attribute__ ((__packed__));

}
#endif /* EVENT_HPP_ */
