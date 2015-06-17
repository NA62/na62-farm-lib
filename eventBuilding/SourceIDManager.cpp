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
uint_fast8_t SourceIDManager::NUMBER_OF_L0_DATA_SOURCES; // Must be greater than 1!!!
uint_fast8_t * SourceIDManager::L0_DATA_SOURCE_IDS; // All sourceIDs participating in L1 (not the CREAM 0x24)
uint_fast8_t * SourceIDManager::L0_DATA_SOURCE_ID_TO_NUM;
uint_fast8_t SourceIDManager::LARGEST_L0_DATA_SOURCE_ID;

uint_fast16_t * SourceIDManager::L0_DATA_SOURCE_ID_TO_PACKNUM; // Expected packets per sourceID
uint_fast16_t * SourceIDManager::L0_DATA_SOURCE_NUM_TO_PACKNUM;
uint_fast16_t SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT; // The sum of all DATA_SOURCE_ID_TO_PACKNUM entries

/*
 * CREAM (LKr and MUV1/2)
 */
uint_fast16_t SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
uint_fast16_t** SourceIDManager::CRATE_AND_CREAM_IDS_TO_LOCAL_ID;
std::pair<uint_fast16_t, uint_fast16_t>* SourceIDManager::LOCAL_ID_TO_CRATE_AND_CREAM_IDS;
std::map<uint_fast16_t, std::vector<uint_fast16_t>> SourceIDManager::CREAM_IDS_BY_CRATE;

uint SourceIDManager::LARGEST_CREAM_CRATE = 0;
uint64_t SourceIDManager::ENABLED_CREAM_CRATES_LUT = 0;
uint_fast32_t* SourceIDManager::ENABLED_CREAMS_BY_CRATE_LUT;

/*
 * LKr
 */
uint_fast16_t SourceIDManager::NUMBER_OF_EXPECTED_LKR_CREAM_FRAGMENTS;

/*
 * MUVs
 */
uint_fast16_t SourceIDManager::MUV_CREAM_CRATE;
uint_fast16_t SourceIDManager::MUV1_NUMBER_OF_FRAGMENTS = 0;
uint_fast16_t SourceIDManager::MUV2_NUMBER_OF_FRAGMENTS = 0;

uint_fast8_t SourceIDManager::TS_SOURCEID_NUM;
bool SourceIDManager::L0TP_ACTIVE = false;

void SourceIDManager::Initialize(const uint_fast16_t timeStampSourceID,
		std::vector<std::pair<int, int> > sourceIDs,
		std::vector<std::pair<int, int> > creamCrates,
		std::vector<std::pair<int, int> > inactiveCreams, int muvCrate) {

	/*
	 * OPTION_DATA_SOURCE_IDS
	 *
	 */
	NUMBER_OF_L0_DATA_SOURCES = sourceIDs.size();

	L0_DATA_SOURCE_IDS = new uint_fast8_t[NUMBER_OF_L0_DATA_SOURCES];
	L0_DATA_SOURCE_NUM_TO_PACKNUM = new uint_fast16_t[NUMBER_OF_L0_DATA_SOURCES];

	bool CreamsActive = false;

	int pos = -1;
	for (auto& pair : sourceIDs) {
		if (pair.first == SOURCE_ID_LKr || pair.first == SOURCE_ID_MUV1
				|| pair.first == SOURCE_ID_MUV2) {
			NUMBER_OF_L0_DATA_SOURCES--;
			CreamsActive = true;
			continue;
		}

		L0_DATA_SOURCE_IDS[++pos] = pair.first;
		L0_DATA_SOURCE_NUM_TO_PACKNUM[pos] = pair.second;
		NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT += pair.second;
	}

	LARGEST_L0_DATA_SOURCE_ID = 0;
	for (int i = 0; i < NUMBER_OF_L0_DATA_SOURCES; i++) {
		if (LARGEST_L0_DATA_SOURCE_ID < L0_DATA_SOURCE_IDS[i]) {
			LARGEST_L0_DATA_SOURCE_ID = L0_DATA_SOURCE_IDS[i];
		}
	}

	L0_DATA_SOURCE_ID_TO_NUM = new uint_fast8_t[LARGEST_L0_DATA_SOURCE_ID + 1];
	L0_DATA_SOURCE_ID_TO_PACKNUM = new uint_fast16_t[LARGEST_L0_DATA_SOURCE_ID + 1];

	memset(L0_DATA_SOURCE_ID_TO_NUM, 0xFF, LARGEST_L0_DATA_SOURCE_ID + 1);

	for (uint_fast8_t i = 0; i < NUMBER_OF_L0_DATA_SOURCES; i++) {
		L0_DATA_SOURCE_ID_TO_NUM[L0_DATA_SOURCE_IDS[i]] = i;
		L0_DATA_SOURCE_ID_TO_PACKNUM[L0_DATA_SOURCE_IDS[i]] =
				L0_DATA_SOURCE_NUM_TO_PACKNUM[i];
	}

	/*
	 * OPTION_CREAM_CRATES
	 *
	 */
	if (CreamsActive) {

		if (creamCrates.size() == 0) {
			throw NA62Error("Option defining CREAM IDs must not be empty!'");
		}

		/*
		 * Check if all inactive CREAMs are listed in the normal cream crate list
		 */
		for (auto& inactivePair : inactiveCreams) {
			uint_fast8_t crateID = inactivePair.first;
			uint_fast8_t CREAMID = inactivePair.second;
			for (auto& pair : creamCrates) {
				if (crateID == pair.first && CREAMID == pair.second) {
					continue;
				}
			}
			throw NA62Error(
					"The CREAM " + std::to_string(CREAMID) + " in crate "
							+ std::to_string(crateID)
							+ " appears in the list of inactive CREAMs but not in the list of available CREAMs");
		}

		NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT = creamCrates.size()
				- inactiveCreams.size();

		NUMBER_OF_EXPECTED_LKR_CREAM_FRAGMENTS =
				NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
		LOCAL_ID_TO_CRATE_AND_CREAM_IDS =
				new std::pair<uint_fast16_t, uint_fast16_t>[NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT];

		/*
		 * Sort by crateID to make sure that the order is LKr->MUV1->MUV2
		 */
		std::sort(creamCrates.begin(), creamCrates.end(),
				[](const std::pair<int, int> & a, const std::pair<int, int> & b) -> bool
				{
					/*
					 * If crates are equal sort by creamID for better visualization of active creams
					 */
					if(a.first==b.first) {
						return a.second < b.second;
					}
					return a.first < b.first;
				});

		std::set<uint_fast8_t> allCrates;
		int creamNum = -1;
		for (auto& pair : creamCrates) {
			uint_fast8_t crateID = pair.first;
			uint_fast8_t CREAMID = pair.second;

			allCrates.insert(crateID);
			for (auto& inactivePair : inactiveCreams) {
				if (crateID == inactivePair.first
						&& CREAMID == inactivePair.second) {
					continue;
				}
			}
			LOCAL_ID_TO_CRATE_AND_CREAM_IDS[++creamNum] = std::make_pair(
					crateID, CREAMID);

			CREAM_IDS_BY_CRATE[crateID].push_back(CREAMID);

			if (crateID > LARGEST_CREAM_CRATE) {
				LARGEST_CREAM_CRATE = crateID;
			}
			ENABLED_CREAM_CRATES_LUT |= 1 << crateID; // Set crateID-th bit to 1
		}

		/*
		 * Write the lookup table for crate+creamID to local cream ID
		 */
		CRATE_AND_CREAM_IDS_TO_LOCAL_ID =
				new uint_fast16_t*[LARGEST_CREAM_CRATE + 1];
		for (uint i = 0; i != LARGEST_CREAM_CRATE + 1; i++) {
			CRATE_AND_CREAM_IDS_TO_LOCAL_ID[i] = nullptr;
		}

		// Add every cream to the LUT
		for (uint localCreamID = 0;
				localCreamID != NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
				localCreamID++) {
			auto crateAndCream = LOCAL_ID_TO_CRATE_AND_CREAM_IDS[localCreamID];
			const auto crateID = crateAndCream.first;
			const auto creamID = crateAndCream.second;

			/*
			 * Initialize the array for the current cream
			 */
			if (CRATE_AND_CREAM_IDS_TO_LOCAL_ID[crateID] == nullptr) {
				uint maxCreamID = 0;
				// Find the largest CREAMID defining the number of elements
				for (auto cID : CREAM_IDS_BY_CRATE[crateID]) {
					if (cID > maxCreamID) {
						maxCreamID = cID;
					}
				}
				CRATE_AND_CREAM_IDS_TO_LOCAL_ID[crateID] =
						new uint_fast16_t[maxCreamID + 1];
			}
			CRATE_AND_CREAM_IDS_TO_LOCAL_ID[crateID][creamID] = localCreamID;
		}

		/*
		 * Write the lookup table for the enabled creams/crates
		 * For every crate set the bit at creamID of ENABLED_CREAMS_BY_CRATE_LUT[crateID] to 1
		 */
		ENABLED_CREAMS_BY_CRATE_LUT = new uint_fast32_t[LARGEST_CREAM_CRATE + 1];
		memset(ENABLED_CREAMS_BY_CRATE_LUT, 0,
				(LARGEST_CREAM_CRATE + 1)
						* sizeof(ENABLED_CREAMS_BY_CRATE_LUT[0]));

		for (auto crateAndCreams : CREAM_IDS_BY_CRATE) {
			uint crateID = crateAndCreams.first;
			for (auto creamID : crateAndCreams.second) {
				ENABLED_CREAMS_BY_CRATE_LUT[crateID] |= 1 << creamID;
			}
		}

		/*
		 * MUV1/2
		 */
		std::vector<uint_fast8_t> allCratesVector(allCrates.begin(),
				allCrates.end());

		MUV_CREAM_CRATE = muvCrate;
		if (muvCrate >= 0) {
			MUV1_NUMBER_OF_FRAGMENTS = CREAM_IDS_BY_CRATE[muvCrate].size();
			NUMBER_OF_EXPECTED_LKR_CREAM_FRAGMENTS -= MUV1_NUMBER_OF_FRAGMENTS;

			if (allCratesVector[allCratesVector.size() - 1] != muvCrate) {
				throw NA62Error(
						"The MUV crate ID must be the largest crateID available which is "
								+ std::to_string(
										allCratesVector[allCratesVector.size()
												- 1]));
			}
			uint minMUV2CreamID = 13;
			for (auto cID : CREAM_IDS_BY_CRATE[muvCrate]) {
				if (cID >= minMUV2CreamID) MUV2_NUMBER_OF_FRAGMENTS++;
			}
            MUV1_NUMBER_OF_FRAGMENTS -= MUV2_NUMBER_OF_FRAGMENTS;
		}

		std::stringstream sstream;
		sstream << "List of activated CREAMs (" << creamCrates.size()
				<< " total):\n";
		for (auto creamsAndCrate : CREAM_IDS_BY_CRATE) {
			sstream << (int) creamsAndCrate.first << ":\t";
			for (auto creamID : creamsAndCrate.second) {
				sstream << creamID << "\t";
			}
			sstream << ENDL;
		}
		LOG_INFO<< ENDL;

	} else {
		LOG_INFO << "There is no LKr SourceID in the sourceID option! Will ignore CREAM ID option" << ENDL;
		NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT = 0;
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
	return L0_DATA_SOURCE_ID_TO_NUM[sourceID] != (uint_fast8_t)0xFF;
}

std::string SourceIDManager::sourceIdToDetectorName(uint_fast8_t sourceID){
	switch (sourceID) {
		case SOURCE_ID_CEDAR: return "CEDAR";
		case SOURCE_ID_GTK: return "GTK";
		case SOURCE_ID_CHANTI: return "CHANTI";
		case SOURCE_ID_LAV: return "LAV";
		case SOURCE_ID_STRAW: return "STRAW";
		case SOURCE_ID_CHOD: return "CHOD";
		case SOURCE_ID_RICH: return "RICH";
		case SOURCE_ID_IRC: return "IRC";
		case SOURCE_ID_LKr: return "LKR";
		case SOURCE_ID_MUV1: return "MUV1";
		case SOURCE_ID_MUV2: return "MUV2";
		case SOURCE_ID_MUV3: return "MUV3";
		case SOURCE_ID_SAC: return "SAC";
		case SOURCE_ID_L0TP: return "L0TP";
		case SOURCE_ID_L1: return "L1";
		case SOURCE_ID_L2: return "L2";
		default: return "UNKNOWN SOURCE ID!";
	}
}

}
/* namespace na62 */
