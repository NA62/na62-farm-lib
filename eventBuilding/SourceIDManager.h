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
#include <sys/types.h>

#define SOURCE_ID_CEDAR 0x04
#define SOURCE_ID_GTK 0x08
#define SOURCE_ID_CHANTI 0x0C
#define SOURCE_ID_LAV 0x10
#define SOURCE_ID_STRAW 0x14
#define SOURCE_ID_CHOD 0x18
//Please notice that newchod SourceId is equal to IRC one
#define SOURCE_ID_NEWCHOD 0x20
#define SOURCE_ID_RICH 0x1C
#define SOURCE_ID_IRC 0x20
#define SOURCE_ID_LKr 0x24
#define SOURCE_ID_MUV1 0x28
#define SOURCE_ID_MUV2 0x2C
#define SOURCE_ID_MUV3 0x30
#define SOURCE_ID_SAC 0x34
#define SOURCE_ID_L0TP 0x40
#define SOURCE_ID_L1 0x44
#define SOURCE_ID_L2 0x48
#define SOURCE_ID_NSTD 0x4C

namespace na62 {

class SourceIDManager {

public:
	/**
	 * Data Source IDs
	 */
	static uint_fast8_t NUMBER_OF_L0_DATA_SOURCES; // Must be greater than 1!!!
	static uint_fast8_t NUMBER_OF_L1_DATA_SOURCES; // Must be greater than 0!!!
	static uint_fast8_t * L0_DATA_SOURCE_IDS; // All sourceIDs participating in L1 (not the CREAM 0x24)
	static uint_fast8_t * L0_DATA_SOURCE_ID_TO_NUM;
	static uint_fast8_t * L1_DATA_SOURCE_IDS; // All sourceIDs participating in L1 (not the CREAM 0x24)
	static uint_fast8_t * L1_DATA_SOURCE_ID_TO_NUM;
	static uint_fast8_t LARGEST_L0_DATA_SOURCE_ID; // ?what for?
	static uint_fast8_t LARGEST_L1_DATA_SOURCE_ID; // ?what for?

	static uint_fast16_t * L0_DATA_SOURCE_ID_TO_PACKNUM; // Expected packets per sourceID
	static uint_fast16_t * L0_DATA_SOURCE_NUM_TO_PACKNUM;
	static uint_fast16_t NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT; // The sum of all DATA_SOURCE_ID_TO_PACKNUM entries

	static uint_fast16_t * L1_DATA_SOURCE_ID_TO_PACKNUM; // Expected packets per sourceID
	static uint_fast16_t * L1_DATA_SOURCE_NUM_TO_PACKNUM;
	static uint_fast16_t NUMBER_OF_EXPECTED_L1_PACKETS_PER_EVENT; // The sum of all DATA_SOURCE_ID_TO_PACKNUM entries

	static uint_fast8_t TS_SOURCEID_NUM;

	static bool L0TP_ACTIVE;

	/**
	 * @param timeStampSourceID The sourceID of the subdetector that should define the timestamp of every event
	 * @param l0sourceIDs A list of pairs of available sourceIDs and the number of frames coming from each sourceID
	 * 	@param l1sourceIDs A list of pairs of available sourceIDs and the number of frames coming from each sourceID
	 */
	static void Initialize(const uint_fast16_t timeStampSourceID,
			std::vector<std::pair<int, int> > l0SourceIDs,
			std::vector<std::pair<int, int> > l1SourceIDs);

	static inline uint_fast16_t getExpectedPacksBySourceNum(
			const uint_fast8_t sourceNum) {
		return L0_DATA_SOURCE_NUM_TO_PACKNUM[sourceNum];
	}

	static inline uint_fast16_t getExpectedPacksBySourceID(const uint_fast8_t sourceID) {
		return L0_DATA_SOURCE_ID_TO_PACKNUM[sourceID];
	}

	static inline uint_fast16_t getExpectedL1PacksBySourceNum(
			const uint_fast8_t sourceNum) {
		return L1_DATA_SOURCE_NUM_TO_PACKNUM[sourceNum];
	}

	static inline uint_fast16_t getExpectedL1PacksBySourceID(const uint_fast8_t sourceID) {
		return L1_DATA_SOURCE_ID_TO_PACKNUM[sourceID];
	}


	/**
	 * Returns true if the CEDAR is activated so that it's data is stored in every event from L1 on
	 */
	static inline bool isCedarActive() {
		return checkL0SourceID(SOURCE_ID_CEDAR);
	}

	/**
	 * Returns true if the GTK is activated so that it's data is stored in every event from L1 on
	 */
	static inline bool isGtkActive() {
		return checkL0SourceID(SOURCE_ID_GTK);
	}

	/**
	 * Returns true if the CHANTI is activated so that it's data is stored in every event from L1 on
	 */
	static inline bool isChantiActive() {
		return checkL0SourceID(SOURCE_ID_CHANTI);
	}

	/**
	 * Returns true if the LAV is activated so that it's data is stored in every event from L1 on
	 */
	static inline bool isLavActive() {
		return checkL0SourceID(SOURCE_ID_LAV);
	}

	/**
	 * Returns true if the STRAW is activated so that it's data is stored in every event from L1 on
	 */
	static inline bool isStrawActive() {
		return checkL0SourceID(SOURCE_ID_STRAW);
	}

	/**
	 * Returns true if the CHOD is activated so that it's data is stored in every event from L1 on
	 */
	static inline bool isChodActive() {
		return checkL0SourceID(SOURCE_ID_CHOD);
	}

	/**
	 * Returns true if the RICH is activated so that it's data is stored in every event from L1 on
	 */
	static inline bool isRhichActive() {
		return checkL0SourceID(SOURCE_ID_RICH);
	}

	/**
	 * Returns true if the IRC is activated so that it's data is stored in every event from L1 on
	 */
	static inline bool isIrcActive() {
		return checkL0SourceID(SOURCE_ID_IRC);
	}

	/**
	 * Returns true if the LKR is activated so that it's data is stored in every event from L2 on
	 */
	static inline bool isLkrActive() {
		return checkL0SourceID(SOURCE_ID_LKr);
	}

	/**
	 * Returns true if the MUV1 is activated so that it's data is stored in every event from L2 on
	 */
	static inline bool isMUV1Active() {
		return checkL0SourceID(SOURCE_ID_MUV1);
	}

	/**
	 * Returns true if the MUV2 is activated so that it's data is stored in every event from L2 on
	 */
	static inline bool isMUV2Active() {
		return checkL0SourceID(SOURCE_ID_MUV2);
	}

	/**
	 * Returns true if the MUV3 is activated so that it's data is stored in every event from L1 on
	 */
	static inline bool isMUV3Active() {
		return checkL0SourceID(SOURCE_ID_MUV3);
	}

	/**
	 * Returns true if the SAC is activated so that it's data is stored in every event from L1 on
	 */
	static inline bool isSacActive() {
		return checkL0SourceID(SOURCE_ID_SAC);
	}

	/**
	 * Returns true if the L0TP is activated so that it's data is stored in every event from L1 on
	 */
	static inline bool isL0TPActive() {
		return checkL0SourceID(SOURCE_ID_L0TP);
	}

	/**
	 * Returns true if the L1  is activated so that it's data is stored in every event from L1 on
	 */
	static inline bool isL1Active() {
		return checkL0SourceID(SOURCE_ID_L1);
	}

	/**
	 * Returns true if the L2  is activated so that it's data is stored in every event from L1 on
	 */
	static inline bool isL2Active() {
		return checkL0SourceID(SOURCE_ID_L2);
	}
	/**
	 * Returns true if the L2  is activated so that it's data is stored in every event from L1 on
	 */
	static inline bool isNSTDActive() {
		return checkL0SourceID(SOURCE_ID_NSTD);
	}


	/*
	 * sourceID must be a valid SourceID! So use checkSourceID if you are not sure!
	 * 0 <= sourceID < L1_LARGEST_DATA_SOURCE_ID
	 */
	static inline uint_fast8_t sourceIDToNum(const uint_fast8_t sourceID) {
		return L0_DATA_SOURCE_ID_TO_NUM[sourceID];
	}

	/*
	 * 0 <= sourceNum < L1_NUMBER_OF_DATA_SOURCES
	 */
	static inline uint_fast8_t sourceNumToID(const uint_fast8_t sourceNum) {
		return L0_DATA_SOURCE_IDS[sourceNum];
	}

	/*
	 * sourceID must be a valid L1 SourceID! So use checkSourceID if you are not sure!
	 */
	static inline uint_fast8_t l1SourceIDToNum(const uint_fast8_t sourceID) {
		return L1_DATA_SOURCE_ID_TO_NUM[sourceID];
	}

	/*
	 * 0 <= sourceNum < L1_NUMBER_OF_DATA_SOURCES
	 */
	static inline uint_fast8_t l1SourceNumToID(const uint_fast8_t sourceNum) {
		return L1_DATA_SOURCE_IDS[sourceNum];
	}

	/*
	 * @return bool <true> if the sourceID is correct, <false> else
	 */
	static bool checkL0SourceID(const uint_fast8_t sourceID);
	/*
	 * @return bool <true> if the sourceID is correct, <false> else
	 */
	static bool checkL1SourceID(const uint_fast8_t sourceID);

	static std::string sourceIdToDetectorName(uint_fast8_t sourceID);
};

} /* namespace na62 */

#endif /* SOURCEIDMANAGER_H_ */
