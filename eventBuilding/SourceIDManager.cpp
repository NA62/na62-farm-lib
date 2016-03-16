/*
 * SourceIDManager.cpp
 *
 *  Created on: Feb 27, 2014
 \*      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "SourceIDManager.h"

#include <algorithm>
#include <vector>
#include <set>

#include <iostream>

#include "../exceptions/NA62Error.h"
#include "../exceptions/BadOption.h"
#include "../options/Options.h"
#include "../options/Logging.h"

namespace na62 {
uint_fast8_t SourceIDManager::NUMBER_OF_L0_DATA_SOURCES = 0; // Must be greater than 1!!!
uint_fast8_t * SourceIDManager::L0_DATA_SOURCE_IDS = 0; // All sourceIDs participating in L1 (not the CREAM 0x24)
uint_fast8_t * SourceIDManager::L0_DATA_SOURCE_ID_TO_NUM = 0;
uint_fast8_t SourceIDManager::LARGEST_L0_DATA_SOURCE_ID = 0;

uint_fast16_t * SourceIDManager::L0_DATA_SOURCE_ID_TO_PACKNUM = 0; // Expected packets per sourceID
uint_fast16_t * SourceIDManager::L0_DATA_SOURCE_NUM_TO_PACKNUM = 0;
uint_fast16_t SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT = 0; // The sum of all DATA_SOURCE_ID_TO_PACKNUM entries

uint_fast8_t SourceIDManager::NUMBER_OF_L1_DATA_SOURCES = 0; // Must be greater than 1!!!
uint_fast8_t * SourceIDManager::L1_DATA_SOURCE_IDS = 0; // All sourceIDs participating in L1 (not the CREAM 0x24)
uint_fast8_t * SourceIDManager::L1_DATA_SOURCE_ID_TO_NUM = 0;
uint_fast8_t SourceIDManager::LARGEST_L1_DATA_SOURCE_ID = 0;

uint_fast16_t * SourceIDManager::L1_DATA_SOURCE_ID_TO_PACKNUM = 0; // Expected packets per sourceID
uint_fast16_t * SourceIDManager::L1_DATA_SOURCE_NUM_TO_PACKNUM = 0;
uint_fast16_t SourceIDManager::NUMBER_OF_EXPECTED_L1_PACKETS_PER_EVENT = 0; // The sum of all DATA_SOURCE_ID_TO_PACKNUM entries

uint_fast8_t SourceIDManager::TS_SOURCEID_NUM;
bool SourceIDManager::L0TP_ACTIVE = false;

void SourceIDManager::Initialize(const uint_fast16_t timeStampSourceID,
		std::vector<std::pair<int, int> > l0sourceIDs,
		std::vector<std::pair<int, int> > l1sourceIDs) {

	/*
	 * OPTION_DATA_SOURCE_IDS
	 *
	 */
	NUMBER_OF_L0_DATA_SOURCES = l0sourceIDs.size();
	L0_DATA_SOURCE_IDS = new uint_fast8_t[NUMBER_OF_L0_DATA_SOURCES];
	L0_DATA_SOURCE_NUM_TO_PACKNUM = new uint_fast16_t[NUMBER_OF_L0_DATA_SOURCES];

	NUMBER_OF_L1_DATA_SOURCES = l1sourceIDs.size();
	L1_DATA_SOURCE_IDS = new uint_fast8_t[NUMBER_OF_L1_DATA_SOURCES];
	L1_DATA_SOURCE_NUM_TO_PACKNUM = new uint_fast16_t[NUMBER_OF_L1_DATA_SOURCES];

	int pos = -1;
	for (auto& pair : l0sourceIDs) {
		L0_DATA_SOURCE_IDS[++pos] = pair.first;
		L0_DATA_SOURCE_NUM_TO_PACKNUM[pos] = pair.second;
		NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT += pair.second;
	}
	pos = -1;
	for (auto& pair : l1sourceIDs) {
		L1_DATA_SOURCE_IDS[++pos] = pair.first;
		L1_DATA_SOURCE_NUM_TO_PACKNUM[pos] = pair.second;
		NUMBER_OF_EXPECTED_L1_PACKETS_PER_EVENT += pair.second;
	}

	LARGEST_L0_DATA_SOURCE_ID = 0;
	for (int i = 0; i < NUMBER_OF_L0_DATA_SOURCES; i++) {
		if (LARGEST_L0_DATA_SOURCE_ID < L0_DATA_SOURCE_IDS[i]) {
			LARGEST_L0_DATA_SOURCE_ID = L0_DATA_SOURCE_IDS[i];
		}
	}

	LARGEST_L1_DATA_SOURCE_ID = 0;
	for (int i = 0; i < NUMBER_OF_L1_DATA_SOURCES; i++) {
		if (LARGEST_L1_DATA_SOURCE_ID < L1_DATA_SOURCE_IDS[i]) {
			LARGEST_L1_DATA_SOURCE_ID = L1_DATA_SOURCE_IDS[i];
		}
	}

	L0_DATA_SOURCE_ID_TO_NUM = new uint_fast8_t[LARGEST_L0_DATA_SOURCE_ID + 1];
	L0_DATA_SOURCE_ID_TO_PACKNUM = new uint_fast16_t[LARGEST_L0_DATA_SOURCE_ID + 1];

	L1_DATA_SOURCE_ID_TO_NUM = new uint_fast8_t[LARGEST_L1_DATA_SOURCE_ID + 1];
	L1_DATA_SOURCE_ID_TO_PACKNUM = new uint_fast16_t[LARGEST_L1_DATA_SOURCE_ID + 1];

	memset(L0_DATA_SOURCE_ID_TO_NUM, 0xFF, LARGEST_L0_DATA_SOURCE_ID + 1);
	memset(L1_DATA_SOURCE_ID_TO_NUM, 0xFF, LARGEST_L1_DATA_SOURCE_ID + 1);

	for (uint_fast8_t i = 0; i < NUMBER_OF_L0_DATA_SOURCES; i++) {
		L0_DATA_SOURCE_ID_TO_NUM[L0_DATA_SOURCE_IDS[i]] = i;
		L0_DATA_SOURCE_ID_TO_PACKNUM[L0_DATA_SOURCE_IDS[i]] =
				L0_DATA_SOURCE_NUM_TO_PACKNUM[i];
	}
	for (uint_fast8_t i = 0; i < NUMBER_OF_L1_DATA_SOURCES; i++) {
		L1_DATA_SOURCE_ID_TO_NUM[L1_DATA_SOURCE_IDS[i]] = i;
		L1_DATA_SOURCE_ID_TO_PACKNUM[L1_DATA_SOURCE_IDS[i]] =
				L1_DATA_SOURCE_NUM_TO_PACKNUM[i];
	}

	L0TP_ACTIVE = SourceIDManager::isL0TPActive();
	TS_SOURCEID_NUM = sourceIDToNum(timeStampSourceID);
	if (!SourceIDManager::checkL0SourceID(timeStampSourceID)) {
		LOG_ERROR<< "The timestamp reference source ID is not part of the L0SourceIDs list" << ENDL;
		exit(1);
	}
}

bool SourceIDManager::checkL0SourceID(const uint_fast8_t sourceID) {
	if (sourceID > LARGEST_L0_DATA_SOURCE_ID) {
		return false;
	}
	return L0_DATA_SOURCE_ID_TO_NUM[sourceID] != (uint_fast8_t) 0xFF;
}

bool SourceIDManager::checkL1SourceID(const uint_fast8_t sourceID) {
	if (sourceID > LARGEST_L1_DATA_SOURCE_ID) {
		return false;
	}
	return L1_DATA_SOURCE_ID_TO_NUM[sourceID] != (uint_fast8_t) 0xFF;
}

std::string SourceIDManager::sourceIdToDetectorName(uint_fast8_t sourceID) {
	switch (sourceID) {
	case SOURCE_ID_CEDAR:
		return "CEDAR";
	case SOURCE_ID_GTK:
		return "GTK";
	case SOURCE_ID_CHANTI:
		return "CHANTI";
	case SOURCE_ID_LAV:
		return "LAV";
	case SOURCE_ID_STRAW:
		return "STRAW";
	case SOURCE_ID_CHOD:
		return "CHOD";
	case SOURCE_ID_RICH:
		return "RICH";
	case SOURCE_ID_IRC:
		return "IRC";
	case SOURCE_ID_LKr:
		return "LKR";
	case SOURCE_ID_MUV1:
		return "MUV1";
	case SOURCE_ID_MUV2:
		return "MUV2";
	case SOURCE_ID_MUV3:
		return "MUV3";
	case SOURCE_ID_SAC:
		return "SAC";
	case SOURCE_ID_L0TP:
		return "L0TP";
	case SOURCE_ID_L1:
		return "L1";
	case SOURCE_ID_L2:
		return "L2";
	case SOURCE_ID_NSTD:
		return "NSTD";
	default:
		return "UNKNOWN SOURCE ID!";
	}
}

}
/* namespace na62 */
