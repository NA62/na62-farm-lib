/*
 * SourceIDManager.cpp
 *
 *  Created on: Feb 27, 2014
 \*      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "SourceIDManager.h"

#include "../exceptions/NA62Error.h"

#ifdef USE_GLOG
#include <glog/logging.h>
#endif
#include <iostream>
#include <vector>

#include "../exceptions/BadOption.h"
#include "../options/Options.h"

namespace na62 {
uint8_t SourceIDManager::NUMBER_OF_L0_DATA_SOURCES; // Must be greater than 1!!!
uint8_t * SourceIDManager::L0_DATA_SOURCE_IDS; // All sourceIDs participating in L1 (not the CREAM 0x24)
uint8_t * SourceIDManager::L0_DATA_SOURCE_ID_TO_NUM;
uint8_t SourceIDManager::LARGEST_L0_DATA_SOURCE_ID;

uint16_t * SourceIDManager::L0_DATA_SOURCE_ID_TO_PACKNUM; // Expected packets per sourceID
uint16_t * SourceIDManager::L0_DATA_SOURCE_NUM_TO_PACKNUM;
uint16_t SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT; // The sum of all DATA_SOURCE_ID_TO_PACKNUM entries

uint16_t SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
std::map<uint16_t, uint16_t> SourceIDManager::CRATE_AND_CREAM_IDS_TO_LOCAL_ID;
std::pair<uint16_t, uint16_t>* SourceIDManager::LOCAL_ID_TO_CRATE_AND_CREAM_IDS;

uint16_t SourceIDManager::TS_SOURCEID;

void SourceIDManager::Initialize(const uint16_t timeStampSourceID,
		std::vector<std::pair<int, int> > sourceIDs,
		std::vector<std::pair<int, int> > creamCrates) {
	TS_SOURCEID = timeStampSourceID;

	/*
	 * OPTION_DATA_SOURCE_IDS
	 *
	 */
	NUMBER_OF_L0_DATA_SOURCES = sourceIDs.size();

	L0_DATA_SOURCE_IDS = new uint8_t[NUMBER_OF_L0_DATA_SOURCES];
	L0_DATA_SOURCE_NUM_TO_PACKNUM = new uint16_t[NUMBER_OF_L0_DATA_SOURCES];

	bool LKrActive = false;

	int pos = -1;
	for (auto pair : sourceIDs) {
		if (pair.first == SOURCE_ID_LKr) {
			NUMBER_OF_L0_DATA_SOURCES--;
			LKrActive = true;
			continue;
		}

		L0_DATA_SOURCE_IDS[++pos] = pair.first;
		L0_DATA_SOURCE_NUM_TO_PACKNUM[pos] = pair.second;
		NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT += pair.second;
	}

	/*
	 * OPTION_CREAM_CRATES
	 *
	 */
	if (LKrActive) {

		if (creamCrates.size() == 0) {
			throw NA62Error("Option defining CREAM IDsmust not be empty!'");
		}

		NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT = creamCrates.size();
		LOCAL_ID_TO_CRATE_AND_CREAM_IDS =
				new std::pair<uint16_t, uint16_t>[NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT];

		std::cout << "List of activated CREAMs: \ncrateID\tSlot\n";
		int creamNum = -1;
		for (auto pair : creamCrates) {
			uint8_t crateID = pair.first;
			uint8_t CREAMID = pair.second;
			CRATE_AND_CREAM_IDS_TO_LOCAL_ID[(crateID << 8) | CREAMID] =
					++creamNum;
			LOCAL_ID_TO_CRATE_AND_CREAM_IDS[creamNum] = std::make_pair(crateID,
					CREAMID);
			std::cout << (int) crateID << "\t" << (int) CREAMID << std::endl;
		}
	} else {
		std::cout
				<< "There is no LKr SourceID in the sourceID option! Will ignore CREAM ID option";
		NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT = 0;
	}

	LARGEST_L0_DATA_SOURCE_ID = 0;
	for (int i = 0; i < NUMBER_OF_L0_DATA_SOURCES; i++) {
		if (LARGEST_L0_DATA_SOURCE_ID < L0_DATA_SOURCE_IDS[i]) {
			LARGEST_L0_DATA_SOURCE_ID = L0_DATA_SOURCE_IDS[i];
		}
	}

	L0_DATA_SOURCE_ID_TO_NUM = new uint8_t[LARGEST_L0_DATA_SOURCE_ID + 1];
	L0_DATA_SOURCE_ID_TO_PACKNUM = new uint16_t[LARGEST_L0_DATA_SOURCE_ID + 1];

	for (uint8_t i = 0; i < NUMBER_OF_L0_DATA_SOURCES; i++) {
		L0_DATA_SOURCE_ID_TO_NUM[L0_DATA_SOURCE_IDS[i]] = i;
		L0_DATA_SOURCE_ID_TO_PACKNUM[L0_DATA_SOURCE_IDS[i]] =
				L0_DATA_SOURCE_NUM_TO_PACKNUM[i];
	}

}

bool SourceIDManager::CheckL0SourceID(const uint8_t sourceID) throw () {
	if (sourceID > LARGEST_L0_DATA_SOURCE_ID) {
		return false;
	}
	uint8_t num = L0_DATA_SOURCE_ID_TO_NUM[sourceID];
	if (sourceID != L0_DATA_SOURCE_IDS[num]) {
		return false;
	}
	return true;
}

}
/* namespace na62 */
