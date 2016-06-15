/*
 * MEPFragment.h
 *
 *  Created on: Nov 14, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef MEPFragment_H_
#define MEPFragment_H_

#include <stdint.h>
#include <boost/noncopyable.hpp>

namespace na62 {
namespace l0 {

class MEP;

/**
 * Defines the structure of a L0 event header within a MEP as defined in table 2 in NA62-11-02.
 * The data is over-allocated so you can access the actual payload by something like
 * char* data = MEPFragmentHdr_ptr+sizeof(MEPFragment_HDR);
 */
struct MEPFragment_HDR {
	uint16_t eventLength_; // Number of bytes of the following event data,	including this header.
	uint8_t eventNumberLSB_;
	uint8_t reserved_ :7;
	uint8_t lastEventOfBurst_ :1; // don't take bool as it will allocate 8 bits!

	uint32_t timestamp_;
}__attribute__ ((__packed__));

class MEPFragment: private boost::noncopyable {
public:
	MEPFragment(MEP* mep, const MEPFragment_HDR * data,
			uint_fast32_t& expectedEventNum);
	virtual ~MEPFragment();

	/**
	 * Number of Bytes of the data including the header (sizeof MEPFragment_HDR)
	 */
	inline uint_fast16_t getDataWithHeaderLength() const {
		return rawData->eventLength_;
	}

	/**
	 * Number of Bytes of the payload data
	 */
	inline uint_fast16_t getPayloadLength() const {
		return rawData->eventLength_ - sizeof(MEPFragment_HDR);
	}

	inline uint_fast32_t getTimestamp() const {
		return rawData->timestamp_;
	}

	inline bool isLastEventOfBurst() const {
		return rawData->lastEventOfBurst_;


	}

	/**
	 * Absolute event number (MSB & LSB)
	 */
	inline uint_fast32_t getEventNumber() const {
		return eventNumber_;
	}

	uint_fast8_t getSourceID() const;

	uint_fast8_t getSourceSubID() const;

	uint_fast8_t getSourceIDNum() const;

	/**
	 * Returns a pointer to the MEP-Buffer at the position where the data of this event starts (including the MEPFragment_HDR!).
	 * From there on you should read only getDataWithHeaderLength() bytes!
	 */
	inline const MEPFragment_HDR* getDataWithMepHeader() const {
		return rawData;
	}

	/**
	 * Returns a pointer to the MEP-Buffer at the position where the payload data of this event starts (excluding the MEPFragment_HDR).
	 * From there on you should read only getPayloadLength() bytes!
	 */
	inline const char* getPayload() const {
		return ((char*) rawData) + sizeof(MEPFragment_HDR);
	}

	inline MEP* getMep() {
		return mep_;
	}
private:
	MEP* mep_;
	const MEPFragment_HDR * rawData;

	const uint_fast32_t eventNumber_;
};

} /* namespace l0 */
} /* namespace na62 */
#endif /* MEPFragment_H_ */
