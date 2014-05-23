/*
 * SourceIDManager.h
 *
 *  Created on: Feb 27, 2014
 \*      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */
#pragma once
#ifndef SOURCEIDMANAGER_H_
#define SOURCEIDMANAGER_H_

#include <vector>
#include <cstdint>
#include <map>
#include <utility>

#define SOURCE_ID_CEDAR 0x04
#define SOURCE_ID_GTK 0x08
#define SOURCE_ID_CHANTI 0x0C
#define SOURCE_ID_LAV 0x10
#define SOURCE_ID_STRAW 0x14
#define SOURCE_ID_CHOD 0x18
#define SOURCE_ID_RICH 0x1C
#define SOURCE_ID_IRC 0x20
#define SOURCE_ID_LKr 0x24
#define SOURCE_ID_MUV 0x28
#define SOURCE_ID_SAC 0x2C
#define SOURCE_ID_L0TP 0x40


namespace na62 {

class SourceIDManager {

public:
	/**
	 * Data Source IDs
	 */
	static uint8_t NUMBER_OF_L0_DATA_SOURCES; // Must be greater than 1!!!
	static uint8_t * L0_DATA_SOURCE_IDS; // All sourceIDs participating in L1 (not the CREAM 0x24)
	static uint8_t * L0_DATA_SOURCE_ID_TO_NUM;
	static uint8_t LARGEST_L0_DATA_SOURCE_ID;

	static uint16_t * L0_DATA_SOURCE_ID_TO_PACKNUM; // Expected packets per sourceID
	static uint16_t * L0_DATA_SOURCE_NUM_TO_PACKNUM;
	static uint16_t NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT; // The sum of all DATA_SOURCE_ID_TO_PACKNUM entries

	static uint16_t TS_SOURCEID;

	/*
	 * All available CREAM crate IDs (the least significant byte of the sourceID within CREAM data packets) as defined
	 * on figure 6 in the LKr-Spec from 07.12.2011
	 *
	 *The Crate IDs and the CREAM IDs will be consecutive numbers!
	 */
	static uint16_t NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
	static std::map<uint16_t, uint16_t> CRATE_AND_CREAM_IDS_TO_LOCAL_ID;
	static std::pair<uint16_t, uint16_t>* LOCAL_ID_TO_CRATE_AND_CREAM_IDS;

	static void Initialize(const uint16_t timeStampSourceID, std::vector<std::pair<int, int> > sourceIDs, std::vector<std::pair<int, int> > creamCrates);

	static inline uint16_t getExpectedPacksByEventNum(
			const uint8_t sourceNum) throw () {
		return L0_DATA_SOURCE_NUM_TO_PACKNUM[sourceNum];
	}

	static inline uint16_t getExpectedPacksByEventID(
			const uint8_t sourceID) throw () {
		return L0_DATA_SOURCE_ID_TO_PACKNUM[sourceID];
	}

	/*
	 * sourceID must be a valid SourceID! So use checkSourceID if you are not sure!
	 * 0 <= sourceID < L1_LARGEST_DATA_SOURCE_ID
	 */
	static inline uint8_t SourceIDToNum(const uint8_t sourceID) throw () {
		return L0_DATA_SOURCE_ID_TO_NUM[sourceID];
	}

	/*
	 * 0 <= sourceNum < L1_NUMBER_OF_DATA_SOURCES
	 */
	static inline uint8_t SourceNumToID(const uint8_t sourceNum) throw () {
		return L0_DATA_SOURCE_IDS[sourceNum];
	}

	/*
	 * @return bool <true> if the sourceID is correct, <false> else
	 */
	static bool CheckL0SourceID(const uint8_t sourceID) throw ();

	/**
	 * Returns the local CREAMID corresponding to the given crateID and CREAM_ID
	 *
	 * The local ID is a consecutive number identifying a CREAM
	 * @see Options::getCrateAndCREAMIDByLocalID
	 */
	static inline uint16_t getLocalCREAMID(const uint8_t crateID,
			const uint8_t CREAM_ID) throw () {
		return CRATE_AND_CREAM_IDS_TO_LOCAL_ID[(crateID << 8) | CREAM_ID];
	}

	/**
	 * Returns a pair containing the crate ID as first and CREAM id as second corresponding to the given localID
	 */
	static inline std::pair<uint8_t, uint8_t> getCrateAndCREAMIDByLocalID(
			const uint16_t localID) throw () {
		return LOCAL_ID_TO_CRATE_AND_CREAM_IDS[localID];
	}

	static inline bool CheckCREAMID(const uint8_t crateID,
			const uint8_t CREAM_ID) throw () {
		std::map<uint16_t, uint16_t>* cratesMap =
				&CRATE_AND_CREAM_IDS_TO_LOCAL_ID;
		if (cratesMap->find((crateID << 8) | CREAM_ID) == cratesMap->end()) {
			return false;
		}
		return true;
	}
};

} /* namespace na62 */

#endif /* SOURCEIDMANAGER_H_ */
