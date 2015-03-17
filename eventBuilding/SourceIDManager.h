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
#define SOURCE_ID_RICH 0x1C
#define SOURCE_ID_IRC 0x20
#define SOURCE_ID_LKr 0x24
#define SOURCE_ID_MUV1 0x28
#define SOURCE_ID_MUV2 0x2C
#define SOURCE_ID_MUV3 0x30
#define SOURCE_ID_SAC 0x34
#define SOURCE_ID_L0TP 0x40

namespace na62 {

class SourceIDManager {

public:
	/**
	 * Data Source IDs
	 */
	static uint_fast8_t NUMBER_OF_L0_DATA_SOURCES; // Must be greater than 1!!!
	static uint_fast8_t * L0_DATA_SOURCE_IDS; // All sourceIDs participating in L1 (not the CREAM 0x24)
	static uint_fast8_t * L0_DATA_SOURCE_ID_TO_NUM;
	static uint_fast8_t LARGEST_L0_DATA_SOURCE_ID;

	static uint_fast16_t * L0_DATA_SOURCE_ID_TO_PACKNUM; // Expected packets per sourceID
	static uint_fast16_t * L0_DATA_SOURCE_NUM_TO_PACKNUM;
	static uint_fast16_t NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT; // The sum of all DATA_SOURCE_ID_TO_PACKNUM entries

	static uint_fast8_t TS_SOURCEID_NUM;

	static bool L0TP_ACTIVE;

	/*
	 * All available CREAM crate IDs (the least significant byte of the sourceID within CREAM data packets) as defined
	 * on figure 6 in the LKr-Spec from 07.12.2011
	 *
	 *The Crate IDs and the CREAM IDs will be consecutive numbers!
	 */
	static uint_fast16_t NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
	static uint_fast16_t** CRATE_AND_CREAM_IDS_TO_LOCAL_ID;
	static std::pair<uint_fast16_t, uint_fast16_t>* LOCAL_ID_TO_CRATE_AND_CREAM_IDS;

	static uint_fast16_t NUMBER_OF_EXPECTED_LKR_CREAM_FRAGMENTS;

	static std::map<uint_fast16_t, std::vector<uint_fast16_t>> CREAM_IDS_BY_CRATE;

	static uint LARGEST_CREAM_CRATE;
	static uint64_t ENABLED_CREAM_CRATES_LUT;
	static uint_fast32_t* ENABLED_CREAMS_BY_CRATE_LUT;

	/*
	 * MUV
	 */
	static uint_fast16_t MUV_CREAM_CRATE;
	static uint_fast16_t MUV2_NUMBER_OF_FRAGMENTS;
	static uint_fast16_t MUV1_NUMBER_OF_FRAGMENTS;

	/**
	 * @param timeStampSourceID The sourceID of the subdetector that should define the timestamp of every event
	 * @param sourceIDs A list of pairs of available sourceIDs and the number of frames coming from each sourceID
	 * 	@param creamCrates A list of pairs with a crateID and a CREAM ID
	 */
	static void Initialize(const uint_fast16_t timeStampSourceID,
			std::vector<std::pair<int, int> > sourceIDs,
			std::vector<std::pair<int, int> > creamCrates,
			std::vector<std::pair<int, int> > inactiveCreams, int muvCrate);

	static inline uint_fast16_t getExpectedPacksBySourceNum(
			const uint_fast8_t sourceNum) {
		return L0_DATA_SOURCE_NUM_TO_PACKNUM[sourceNum];
	}

	static inline uint_fast16_t getExpectedPacksBySourceID(const uint_fast8_t sourceID) {
		return L0_DATA_SOURCE_ID_TO_PACKNUM[sourceID];
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
	 * @return bool <true> if the sourceID is correct, <false> else
	 */
	static bool checkL0SourceID(const uint_fast8_t sourceID);

	/**
	 * Returns the local CREAMID corresponding to the given crateID and CREAM_ID
	 *
	 * The local ID is a consecutive number identifying a CREAM
	 * @see Options::getCrateAndCREAMIDByLocalID
	 */
	static inline uint_fast16_t getLocalCREAMID(const uint_fast8_t crateID,
			const uint_fast8_t CREAM_ID) {
		return CRATE_AND_CREAM_IDS_TO_LOCAL_ID[crateID][CREAM_ID];
	}

	/**
	 * Returns a pair containing the crate ID as first and CREAM id as second corresponding to the given localID
	 */
	static inline std::pair<uint_fast8_t, uint_fast8_t> getCrateAndCREAMIDByLocalID(
			const uint_fast16_t localID) {
		return LOCAL_ID_TO_CRATE_AND_CREAM_IDS[localID];
	}

	static inline bool checkCREAMID(const uint_fast8_t crateID,
			const uint_fast8_t CREAM_ID) {

		return CREAM_ID < 32 && (ENABLED_CREAM_CRATES_LUT & (1L << crateID)) // check crate
				&& (ENABLED_CREAMS_BY_CRATE_LUT[crateID] & (1 << CREAM_ID)); // check CREAM of this crate
	}

	static std::string sourceIdToDetectorName(uint_fast8_t sourceID);
};

} /* namespace na62 */

#endif /* SOURCEIDMANAGER_H_ */
