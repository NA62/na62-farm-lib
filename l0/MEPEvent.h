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
 */
struct MEPEVENT_RAW_HDR {
	uint16_t eventLength_;
	uint8_t eventNumberLSB_;
	uint8_t reserved_ :7;
	uint8_t lastEventOfBurst_ :1; // don't take bool as it will allocate 8 bits!
	uint32_t timestamp_;
}__attribute__ ((__packed__));

class MEPEvent {
public:
	MEPEvent(MEP* mep, const char * data, uint32_t& expectedEventNum);
	virtual ~MEPEvent();

	inline const uint16_t getEventLength() const {
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
	 * Returns a pointer to the MEP-Buffer at the position where the data of this event starts (excluding header!).
	 * From there on you should read only getEventLength()-sizeof(struct MEPEVENT_RAW_HDR) bytes!
	 */
	inline const char* getData() const {
		return data_ + sizeof(struct MEPEVENT_RAW_HDR);
	}

	/**
	 * Returns a pointer to the MEP-Buffer at the position where the data of this event starts (including header!).
	 * From there on you should read only getEventLength() bytes!
	 */
	inline const char* getDataWithHeader() const {
		return data_;
	}

	inline MEP* getMep() {
		return mep_;
	}
private:
	MEP* mep_;
	const struct MEPEVENT_RAW_HDR * rawData;

	const uint32_t eventNumber_;
	const char* data_;
};

} /* namespace l0 */
} /* namespace na62 */
#endif /* MEPEVENT_H_ */
