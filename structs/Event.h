/*
 * Event.hpp
 *
 *  Created on: Mar 7, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
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

#define TRIGGER_L0_PULSER_GTK 0x2c
#define TRIGGER_L0_PEDESTAL_LKR 0x2d
#define TRIGGER_L0_CALIBRATION1_LKR 0x30
#define TRIGGER_L0_CALIBRATION2_LKR 0x31
#define TRIGGER_L0_CALIBRATION3_LKR 0x32
#define TRIGGER_L0_CALIBRATION4_LKR 0x33

#define TRIGGER_L1_BYPASS 0x20
#define TRIGGER_L2_BYPASS 0x20

#define TRIGGER_L1_SPECIAL 0xFF
#define TRIGGER_L2_SPECIAL 0xFF
#define TRIGGER_L1_SPECIAL_GTK 0xEE
#define TRIGGER_L2_SPECIAL_GTK 0xEE

namespace na62 {
/**
 * Pointer to the payload of the given sourceID within an event
 */
struct EVENT_DATA_PTR {
	uint32_t offset :24; // Number of 4B-Words from the beginning of the Event
	uint8_t sourceID;
};

/*
 * Header of a completed event as it will be stored on disk
 *
 */
struct EVENT_HDR {
	uint32_t eventNum :24;
	uint8_t formatVersion;

	uint32_t length; // number of 4B-words
	uint32_t burstID;
	uint32_t timestamp;

	uint32_t triggerWord :24;
	uint8_t reserved1;

	uint8_t fineTime;
	uint8_t numberOfDetectors;
	uint16_t reserved2;

	uint32_t processingID;

	uint32_t SOBtimestamp;

	/**
	 * Returns the pointers from the pointer table to the data of all source IDs. Use it as following:
	 * EVENT_DATA_PTR* sourceIdAndOffsets = event->getDataPointer();
	 * for(int sourceNum=0; sourceNum!=event->numberOfDetectors; sourceNum++){
	 * 	EVENT_DATA_PTR* sourceIdAndOffset = sourceIdAndOffsets[sourceNum];
	 * }
	 */
	EVENT_DATA_PTR* getDataPointer() {
		return reinterpret_cast<EVENT_DATA_PTR*>(((char*) this)
				+ sizeof(EVENT_HDR));
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
	uint16_t dataBlockSize;
	uint8_t sourceSubID;
	uint8_t reserved;
	uint32_t timestamp;
}__attribute__ ((__packed__));

struct EVENT_TRAILER {
	uint32_t eventNum :24;
	uint8_t reserved;
}__attribute__ ((__packed__));

}
#endif /* EVENT_HPP_ */
