/*
 * MEPEvent.h
 *
 *  Created on: Nov 14, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef MEPEVENT_H_
#define MEPEVENT_H_

#include <stdint.h>

namespace na62 {
namespace l0 {

class MEP;

/**
 * Defines the structure of a L0 event header within a MEP as defined in table 2 in NA62-11-02.
 * The data is over-allocated so you can access the actual payload by something like
 * char* data = mepeventHdr_ptr+sizeof(MEPEVENT_HDR);
 */
struct MEPEVENT_HDR {
	uint16_t eventLength_; // Number of bytes of the following event data,	including this header.
	uint8_t eventNumberLSB_;
	uint8_t reserved_ :7;
	uint8_t lastEventOfBurst_ :1; // don't take bool as it will allocate 8 bits!
	uint32_t timestamp_;
}__attribute__ ((__packed__));

class MEPEvent {
public:
	MEPEvent(MEP* mep, const MEPEVENT_HDR * data, uint32_t& expectedEventNum);
	virtual ~MEPEvent();

	/**
	 * Number of Bytes of the data including the header (sizeof MEPEVENT_HDR)
	 */
	inline const uint16_t getDataLength() const {
		return rawData->eventLength_;
	}

	inline const uint32_t getTimestamp() const {
		return rawData->timestamp_;
	}

	inline const bool isLastEventOfBurst() const {
		return rawData->lastEventOfBurst_;
	}

	/**
	 * Absolute event number (MSB & LSB)
	 */
	inline const uint32_t getEventNumber() const {
		return eventNumber_;
	}

	const uint8_t getSourceID() const;

	const uint8_t getSourceSubID() const;

	const uint8_t getSourceIDNum() const;

	/**
	 * Returns a pointer to the MEP-Buffer at the position where the data of this event starts (including the header!).
	 * From there on you should read only getEventLength() bytes!
	 */
	inline const MEPEVENT_HDR* getData() const {
		return rawData;
	}

	inline const MEP* getMep() {
		return mep_;
	}
private:
	MEP* mep_;
	const struct MEPEVENT_HDR * rawData;

	const uint32_t eventNumber_;
};

} /* namespace l0 */
} /* namespace na62 */
#endif /* MEPEVENT_H_ */
