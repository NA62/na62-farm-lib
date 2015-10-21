/*
 * LkrFragment.h
 *
 *  Created on: Nov 14, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef LkrFragment_H_
#define LkrFragment_H_

#include <boost/noncopyable.hpp>
#include <netinet/in.h>
#include <cstdint>
#include "../exceptions/NA62Error.h"

#define __USE_BIG_ENDIAN_FOR_LKR_EVENTS

namespace na62 {
namespace cream {

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

class LkrFragment: boost::noncopyable {
public:
	LkrFragment(const char * data, const uint_fast16_t& dataLength, const char* etherFrame) throw (NA62Error);
	virtual ~LkrFragment();

	inline uint_fast32_t getEventLength() const {
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
		return (ntohl(rawData->numberOf4BWords) >> 8) * 4;
#else
		return rawData->numberOf4BWords*4;
#endif
	}

	inline uint_fast32_t getEvent4BWords() const {
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
		return ntohl(rawData->numberOf4BWords >> 8);
#else
		return rawData->numberOf4BWords;
#endif
	}

	/**
	 * Absolute event number (MSB & LSB)
	 */
	inline uint_fast32_t getEventNumber() const {
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
		return ntohl(rawData->eventNumber) >> 8;
#else
		return rawData->eventNumber;
#endif
	}

	inline uint_fast8_t getCrateID() const {
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
		// skip the lowest 5 bits as these are the CREMID
		return (uint_fast8_t) (ntohs(rawData->sourceCrateCREAMID) >> 5) & 63;
#else
		return rawData->sourceCrateID;
#endif
	}

	inline uint_fast8_t getCREAMID() const {
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
		// The CREAMID is in the lowest 5(which is 31) bits
		return (uint_fast8_t) (ntohs(rawData->sourceCrateCREAMID) & 31);
#else
		return rawData->sourceCREAMID;
#endif
	}

	inline uint_fast32_t getTimestamp() const {
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
		return ntohl(rawData->timestamp);
#else
		return rawData->timestamp;
#endif
	}
	/**
	 * Returns a pointer to the LKR-Buffer at the position where the data of this event starts (excluding header!).
	 * From there on you should read only getEventLength()-sizeof(LKR_EVENT_RAW_HDR) bytes!
	 */
	inline const char* getData() const {
		return data_ + sizeof(LKR_EVENT_RAW_HDR);
	}

	/**
	 * Returns a pointer to the LKR-Buffer at the position where the data of this event starts (including header!).
	 * From there on you should read only getEventLength() bytes!
	 */
	inline const char* getDataWithHeader() const {
		return data_;
	}

	inline uint_fast16_t getCrateCREAMID() {
		return generateCrateCREAMID(getCrateID(), getCREAMID());
	}

	inline const char* getEtherFrame(){
		return etherFrame_;
	}

	static inline uint_fast16_t generateCrateCREAMID(const uint_fast8_t crateID, const uint_fast8_t CREAMID) {
#ifdef __USE_BIG_ENDIAN_FOR_LKR_EVENTS
		// The CREAMID is in the lowest 5(which is 31) bits
		return htons((crateID << 8) | CREAMID);
#else
		return (crateID<<8) | CREAMID;
#endif
	}

private:
	const LKR_EVENT_RAW_HDR * rawData;

	const char *data_;
	const char* etherFrame_;
};

} /* namespace cream */
} /* namespace na62 */
#endif /* EVENT_H_ */
