/*
 * Event.hpp
 *
 *  Created on: Mar 7, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once

#include <cstdint>

namespace na62 {

/*
 * Header of a burst File
 */
struct BURST_HDR {
	uint32_t fileFormatVersion :24;
	uint8_t zero; // must be zero to distinguish between BURST_HDR and old burst file format without header (events have their format version here)

	uint32_t numberOfEvents;

	uint32_t runID;

	uint32_t burstID;

	/*
	 * Returns a pointer to the table storing the eventNumbers of all events stored in this file (contains numberOfEvents elements)
	 * The Nth entry stores the event number of the Nth event stored
	 */
	uint32_t* getEventNumbers() {
		return reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(this)
				+ sizeof(BURST_HDR));
	}

	/*
	 * Returns a pointer to the table storing the trigger type words of all events stored in this file (contains numberOfEvents elements)
	 * The Nth entry stores the trigger type word of the Nth event stored
	 */
	uint32_t* getEventTriggerTypeWords() {
		return reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(this)
				+ sizeof(BURST_HDR) + numberOfEvents * sizeof(uint32_t));
	}

	/*
	 * Returns a pointer to the table storing the offsets of all events (contains numberOfEvents elements)
	 * The Nth entry stores the number of 4 byte words you have to jump from the beginning of the file
	 * to reach the beginning of the Nth event stored
	 */
	uint32_t* getEventOffsets() {
		return reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(this)
				+ sizeof(BURST_HDR) + 2 * (numberOfEvents * sizeof(uint32_t)));
	}

	uint getHeaderSize() {
		return sizeof(BURST_HDR) + 3 * numberOfEvents * sizeof(uint32_t);
	}

	static uint calculateHeaderSize(uint numberOfEvents) {
		return sizeof(BURST_HDR) + 3 * numberOfEvents * sizeof(uint32_t);
	}

}__attribute__ ((__packed__));

}
