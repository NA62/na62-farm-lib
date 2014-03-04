/*
 * SourceIDManager.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: root
 */

#include "SourceIDManager.h"

#include <glog/logging.h>
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

void SourceIDManager::Initialize() {
	TS_SOURCEID = Options::GetInt(OPTION_TS_SOURCEID);

	/*
	 * OPTION_DATA_SOURCE_IDS
	 *
	 */
	auto SourceIDs = Options::GetIntPairList(OPTION_DATA_SOURCE_IDS);
	NUMBER_OF_L0_DATA_SOURCES = SourceIDs.size();

	bool LKrActive = false;

	int pos = -1;
	for (auto pair : SourceIDs) {
		if (pair.first == LKR_SOURCE_ID) {
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
		auto pairs = Options::GetIntPairList(OPTION_CREAM_CRATES);

		if (pairs.size() == 0) {
			throw BadOption(OPTION_CREAM_CRATES, "Must not be empty!'");
		}

		NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT = pairs.size();
		LOCAL_ID_TO_CRATE_AND_CREAM_IDS =
				new std::pair<uint16_t, uint16_t>[NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT];

		int creamNum = -1;
		for (auto pair : pairs) {
			uint8_t crateID = pair.first;
			uint8_t CREAMID = pair.second;
			CRATE_AND_CREAM_IDS_TO_LOCAL_ID[(crateID << 8) | CREAMID] =
					++creamNum;
			LOCAL_ID_TO_CRATE_AND_CREAM_IDS[creamNum] = std::make_pair(crateID,
					CREAMID);
		}
	} else {
		LOG(INFO)<<"There is no LKr SourceID in --" << OPTION_DATA_SOURCE_IDS
		<< "! Will ignore --" << OPTION_CREAM_CRATES;
		NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT = 0;
	}

}

}
/* namespace na62 */
