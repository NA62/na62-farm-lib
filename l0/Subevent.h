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

#include "../eventBuilding/SourceIDManager.h"
#include "MEPFragment.h"

namespace na62 {
namespace l0 {

class Subevent: private boost::noncopyable {
public:
	Subevent(const uint16_t expectedPacketsNum);
	virtual ~Subevent();

	void destroy();

	/**
	 * If the Subevent is not complete yet the given fragment will be stored and true is returned.
	 *
	 * Otherwise false is returned
	 *
	 */
	inline bool addFragment(MEPFragment* fragment) {
		uint16_t oldNumberOfFragments = eventPartCounter.fetch_add(1);

		if (oldNumberOfFragments
				>= SourceIDManager::getExpectedPacksBySourceID(
						fragment->getSourceID())) {
			/*
			 * when more fragments are received than expected: decrement the counter back to the old value
			 * We have to check >= as it might be > in case of a high rate where another thread could already
			 * have incremented it without decrementing it yet
			 */
			eventPartCounter--;
			return false;
		}

		eventFragments[oldNumberOfFragments] = fragment;
		return true;
	}

	/**
	 * Returns all fragments of this Subevent
	 * @return A pointer to an array of all received MEPFragment pointers
	 */
	inline MEPFragment ** getEventFragments() {
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
	inline MEPFragment* getFragment(uint16_t eventPartNumber) {
		return eventFragments[eventPartNumber];
	}

	/**
	 * Returns the number of received subevent fragments
	 *
	 * @return The number of subevent fragments received
	 */
	inline uint16_t getNumberOfFragments() {
		return eventPartCounter;
	}

private:
	uint16_t ExpectedPacketsNum;
	MEPFragment ** eventFragments;
	std::atomic<uint16_t> eventPartCounter;
};

} /* namespace l0 */
} /* namespace na62 */
#endif /* SUBEVENT_H_ */
