
#pragma once
#ifndef SUBEVENT_H_
#define SUBEVENT_H_

#include <boost/noncopyable.hpp>
#include <atomic>
#include <cstdbool>
#include <cstdint>
#include <iostream>
#include <set>
#include "MEPFragment.h"

namespace na62 {
namespace l0 {



/**
 * Defines the structure of a L0 event header within a MEP as defined in table 2 in NA62-11-02.
 * The data is over-allocated so you can access the actual payload by something like
 * char* data = MEPFragmentHdr_ptr+sizeof(MEPFragment_HDR);
 */
class Subevent: private boost::noncopyable {
public:
	Subevent(const uint_fast16_t expectedPacketsNumD);
	virtual ~Subevent();

	void destroy();

	/**
	 * If the Subevent is not complete yet the given fragment will be stored and true is returned.
	 *
	 * Otherwise false is returned
	 *
	 */
	inline bool addFragment(MEPFragment_HDR* fragment) {
		uint_fast16_t oldNumberOfFragments = fragmentCounter.fetch_add(1);

//		if (oldNumberOfFragments
//				>= SourceIDManager::getExpectedPacksBySourceID(
//						fragment->getSourceID())) {
//			/*
//			 * when more fragments are received than expected: decrement the counter back to the old value
//			 * We have to check >= as it might be > in case of a high rate where another thread could already
//			 * have incremented it without decrementing it yet
//			 */
//			fragmentCounter--;
//			return false;
//		}

		eventFragments[oldNumberOfFragments]->setData(fragment);
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

	void reset() {
		uint_fast16_t NumberOfFragments = fragmentCounter.fetch_add(1);
	    for (uint index = 0; index < NumberOfFragments; ++index) {
	        eventFragments[index]->reset();
	    }
	    fragmentCounter = 0;
	    //sourceID = 0;
	}

private:
	const uint_fast16_t expectedPacketsNum;
	const uint_fast8_t sourceID;
	std::atomic<uint_fast16_t> fragmentCounter;
	MEPFragment ** eventFragments;
};

} /* namespace l0 */
} /* namespace na62 */
#endif /* SUBEVENT_H_ */
