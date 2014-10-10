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

#ifdef USE_GLOG
#include <glog/logging.h>
#endif
#include <iostream>

#include "../exceptions/NA62Error.h"
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

/*
 * CREAM (LKr and MUV1/2)
 */
uint16_t SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
std::map<uint16_t, uint16_t> SourceIDManager::CRATE_AND_CREAM_IDS_TO_LOCAL_ID;
std::pair<uint16_t, uint16_t>* SourceIDManager::LOCAL_ID_TO_CRATE_AND_CREAM_IDS;
std::map<uint16_t, std::vector<uint16_t>> SourceIDManager::CREAM_IDS_BY_CRATE;

/*
 * LKr
 */
uint16_t SourceIDManager::NUMBER_OF_EXPECTED_LKR_CREAM_FRAGMENTS;

/*
 * MUVs
 */
uint16_t SourceIDManager::MUV_CREAM_CRATE;
uint16_t SourceIDManager::MUV1_NUMBER_OF_FRAGMENTS = 0;
uint16_t SourceIDManager::MUV2_NUMBER_OF_FRAGMENTS = 0;

uint16_t SourceIDManager::TS_SOURCEID;

void SourceIDManager::Initialize(const uint16_t timeStampSourceID,
		std::vector<std::pair<int, int> > sourceIDs,
		std::vector<std::pair<int, int> > creamCrates,
		std::vector<std::pair<int, int> > inactiveCreams, int muvCrate) {
	TS_SOURCEID = timeStampSourceID;

	/*
	 * OPTION_DATA_SOURCE_IDS
	 *
	 */
	NUMBER_OF_L0_DATA_SOURCES = sourceIDs.size();

	L0_DATA_SOURCE_IDS = new uint8_t[NUMBER_OF_L0_DATA_SOURCES];
	L0_DATA_SOURCE_NUM_TO_PACKNUM = new uint16_t[NUMBER_OF_L0_DATA_SOURCES];

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

	L0_DATA_SOURCE_ID_TO_NUM = new uint8_t[LARGEST_L0_DATA_SOURCE_ID + 1];
	L0_DATA_SOURCE_ID_TO_PACKNUM = new uint16_t[LARGEST_L0_DATA_SOURCE_ID + 1];

	for (uint8_t i = 0; i < NUMBER_OF_L0_DATA_SOURCES; i++) {
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
		 * Check if all inactive CREAMs are listed in the normal cream create list
		 */
		for (auto& inactivePair : inactiveCreams) {
			uint8_t crateID = inactivePair.first;
			uint8_t CREAMID = inactivePair.second;
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
		LOCAL_ID_TO_CRATE_AND_CREAM_IDS =
				new std::pair<uint16_t, uint16_t>[NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT];

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

		std::set<uint8_t> allCrates;
		int creamNum = -1;
		for (auto& pair : creamCrates) {
			uint8_t crateID = pair.first;
			uint8_t CREAMID = pair.second;

			allCrates.insert(crateID);
			for (auto& inactivePair : inactiveCreams) {
				if (crateID == inactivePair.first
						&& CREAMID == inactivePair.second) {
					continue;
				}
			}
			CRATE_AND_CREAM_IDS_TO_LOCAL_ID[(crateID << 8) | CREAMID] =
					++creamNum;
			LOCAL_ID_TO_CRATE_AND_CREAM_IDS[creamNum] = std::make_pair(crateID,
					CREAMID);

			CREAM_IDS_BY_CRATE[crateID].push_back(CREAMID);
		}

		/*
		 * MUV1/2
		 */
		std::vector<uint8_t> allCratesVector(allCrates.begin(),
				allCrates.end());

		MUV_CREAM_CRATE = muvCrate;
		if (muvCrate >= 0) {
			NUMBER_OF_EXPECTED_LKR_CREAM_FRAGMENTS--;
			MUV1_NUMBER_OF_FRAGMENTS = CREAM_IDS_BY_CRATE[muvCrate].size();
			/*
			 * TODO: Impelment the separation of MUV1/MUV2
			 */
			if (allCratesVector[allCratesVector.size() - 2] != muvCrate) {
				throw NA62Error(
						"The MUV1 crate ID must be the second largest crateID available");
			}
		}

		std::cout << "List of activated CREAMs (" << creamCrates.size()
				<< " total):\n";
		for (auto creamsAndCrate : CREAM_IDS_BY_CRATE) {
			std::cout << (int) creamsAndCrate.first << ":\t";
			for (auto creamID : creamsAndCrate.second) {
				std::cout << creamID << "\t";
			}
			std::cout << std::endl;
		}

	} else {
		std::cout
				<< "There is no LKr SourceID in the sourceID option! Will ignore CREAM ID option"
				<< std::endl;
		NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT = 0;
	}
}

bool SourceIDManager::CheckL0SourceID(const uint8_t sourceID) {
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
