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
#include <cstdint>

namespace na62 {
namespace l0 {
class MEPFragment;
} /* namespace l0 */
} /* namespace na62 */

namespace na62 {
namespace l0 {

class Subevent: private boost::noncopyable {
public:
	Subevent(const uint16_t expectedPacketsNum);
	virtual ~Subevent();

	void destroy();
	inline void addFragment(MEPFragment* eventPart) throw () {
		eventFragments[eventPartCounter++] = eventPart;
	}

	/**
	 * Returns all fragments of this Subevent
	 * @return A pointer to an array of all received MEPFragment pointers
	 */
	inline MEPFragment ** getEventFragments() throw () {
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
	inline MEPFragment* getFragment(uint16_t eventPartNumber) throw () {
		return eventFragments[eventPartNumber];
	}

	/**
	 * Returns the number of received subevent fragments
	 *
	 * @return The number of subevent fragments received
	 */
	inline uint16_t getNumberOfFragments() throw () {
		return eventPartCounter;
	}

private:
	uint16_t ExpectedPacketsNum;
	MEPFragment ** eventFragments;
	uint16_t eventPartCounter;
};

} /* namespace l0 */
} /* namespace na62 */
#endif /* SUBEVENT_H_ */
