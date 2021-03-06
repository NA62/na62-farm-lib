/*
 * Subevent.h
 *
 *  Created on: Jan 25, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef SUBEVENT_H_
#define SUBEVENT_H_

#include <boost/noncopyable.hpp>
#include <atomic>
#include <cstdbool>
#include <cstdint>
#include <iostream>
#include <set>

#include "../eventBuilding/SourceIDManager.h"
#include "MEPFragment.h"

namespace na62 {
namespace l0 {

class Subevent: private boost::noncopyable {
public:
	Subevent(const uint_fast16_t expectedPacketsNum, const uint_fast8_t sourceID);
	virtual ~Subevent();

	void destroy();

	/**
	 * If the Subevent is not complete yet the given fragment will be stored and true is returned.
	 *
	 * Otherwise false is returned
	 *
	 */
	inline bool addFragment(MEPFragment* fragment) {
		uint_fast16_t oldNumberOfFragments = fragmentCounter.fetch_add(1);

		if (oldNumberOfFragments
				>= SourceIDManager::getExpectedPacksBySourceID(
						fragment->getSourceID())) {
			/*
			 * when more fragments are received than expected: decrement the counter back to the old value
			 * We have to check >= as it might be > in case of a high rate where another thread could already
			 * have incremented it without decrementing it yet
			 */
			fragmentCounter--;
			return false;
		}

		eventFragments[oldNumberOfFragments] = fragment;
		return true;
	}

	/**
	 * Returns all fragments of this Subevent
	 * @return A pointer to an array of all received MEPFragment pointers
	 */
	inline MEPFragment ** getEventFragments() const {
		return eventFragments;
	}

	/**
	 * Returns the Nth event fragment that has been received
	 *
	 * @param eventPartNumber
	 * 						The number of the requested fragment (N). N must be smaller
	 * 						than  getNumberOfParts()
	 * @return The Nth fragment received
	 */
	inline MEPFragment* getFragment(uint_fast16_t eventPartNumber) const {
		return eventFragments[eventPartNumber];
	}

	/**
	 * Returns all missing source sub IDs. This only works correctly if the enabled sub IDs are consecutive numbers from 0 to ExpectedPacketsNum-1
	 */
	inline std::vector<uint> getMissingSourceSubIds() const {
		std::vector<uint> missingSubIDs;

		std::set<uint> receivedSubIDs;
		for (uint i = 0; i != getNumberOfFragments(); i++) {
			receivedSubIDs.insert(eventFragments[i]->getSourceSubID());
		}

		// Check which subIDs are missing
		for (uint i = 0; i != expectedPacketsNum; i++) {
			if (receivedSubIDs.find(i) == receivedSubIDs.end()) {
				missingSubIDs.push_back(i);
			}
		}

		return missingSubIDs;
	}

	/**
	 * Returns the number of received subevent fragments
	 *
	 * @return The number of subevent fragments received
	 */
	inline uint_fast16_t getNumberOfFragments() const {
		return fragmentCounter;
	}

	uint_fast16_t getNumberOfExpectedFragments() const {
		return expectedPacketsNum;
	}
	uint8_t getSourceID() {
		return sourceID;
	}
private:
	const uint_fast16_t expectedPacketsNum;
	const uint_fast8_t sourceID;
	MEPFragment ** eventFragments;
	std::atomic<uint_fast16_t> fragmentCounter;
};

} /* namespace l0 */
} /* namespace na62 */
#endif /* SUBEVENT_H_ */
