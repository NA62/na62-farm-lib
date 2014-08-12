/*
 * LkrFragment.h
 *
 *  Created on: Nov 14, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef LkrFragment_H_
#define LkrFragment_H_

#include <netinet/in.h>
#include <cstdint>
#include "../exceptions/NA62Error.h"

#define __USE_BIG_ENDIAN_FOR_LKR_EVENTS

namespace na62 {
namespace cream {

class LKRMEP;

/**
 * Defines the structure of a L1 CREAM event header as defined in the LKr spec.
 */
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
struct LKR_EVENT_RAW_HDR {
	uint8_t LKRsourceID;
	uint32_t eventNumber :24;

	uint8_t reserved;
	uint32_t numberOf4BWords :24;

	uint32_t timestamp;

	uint16_t reserved2;
	uint16_t sourceCrateCREAMID;
//	uint16_t reserved3 :5;
}__attribute__ ((__packed__));
#else
struct LKR_EVENT_RAW_HDR {
	uint32_t eventNumber :24;
	uint8_t LKRsourceID;
	uint32_t numberOf4BWords :24;
	uint8_t reserved;
	uint32_t timestamp;
	uint8_t sourceCREAMID:5;
	uint16_t sourceCrateID:11;
	uint16_t reserved2;
}__attribute__ ((__packed__));
#endif

class LkrFragment {
public:
	LkrFragment(LKRMEP* mep, const char * data, const uint16_t& dataLength) throw (NA62Error);
	virtual ~LkrFragment();

	inline const uint32_t getEventLength() const {
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
		return (ntohl(rawData->numberOf4BWords) >> 8) * 4;
#else
		return rawData->numberOf4BWords*4;
#endif
	}

	inline const uint32_t getEvent4BWords() const {
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
		return ntohl(rawData->numberOf4BWords >> 8);
#else
		return rawData->numberOf4BWords;
#endif
	}

	/**
	 * Absolute event number (MSB & LSB)
	 */
	inline const uint32_t getEventNumber() const {
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
		return ntohl(rawData->eventNumber) >> 8;
#else
		return rawData->eventNumber;
#endif
	}

	inline const uint8_t getCrateID() const {
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
		// skip the lowest 5 bits as these are the CREMID
		return (uint8_t) (ntohs(rawData->sourceCrateCREAMID) >> 5) & 63;
#else
		return rawData->sourceCrateID;
#endif
	}

	inline const uint8_t getCREAMID() const {
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
		// The CREAMID is in the lowest 5(which is 31) bits
		return (uint8_t) (ntohs(rawData->sourceCrateCREAMID) & 31);
#else
		return rawData->sourceCREAMID;
#endif
	}

	inline const uint32_t getTimestamp() const {
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
		return ntohl(rawData->timestamp);
#else
		return rawData->timestamp;
#endif
	}
	/**
	 * Returns a pointer to the LKR-Buffer at the position where the data of this event starts (excluding header!).
	 * From there on you should read only getEventLength()-sizeof(struct LKR_EVENT_RAW_HDR) bytes!
	 */
	inline const char* getData() const {
		return data_ + sizeof(struct LKR_EVENT_RAW_HDR);
	}

	/**
	 * Returns a pointer to the LKR-Buffer at the position where the data of this event starts (including header!).
	 * From there on you should read only getEventLength() bytes!
	 */
	inline const char* getDataWithHeader() const {
		return data_;
	}

	inline const uint16_t getCrateCREAMID() {
		return generateCrateCREAMID(getCrateID(), getCREAMID());
	}

	static inline const uint16_t generateCrateCREAMID(const uint8_t crateID, const uint8_t CREAMID) {
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
		// The CREAMID is in the lowest 5(which is 31) bits
		return htons((crateID << 8) | CREAMID);
#else
		return (crateID<<8) | CREAMID;
#endif
	}

	const LKRMEP* getMep() const {
		return mep_;
	}

private:
	LKRMEP* mep_;
	const struct LKR_EVENT_RAW_HDR * rawData;

	const char *data_;
};

} /* namespace cream */
} /* namespace na62 */
#endif /* EVENT_H_ */
