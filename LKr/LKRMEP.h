/*
 * Header.h
 *
 *  Created on: Sep 21, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef LKRMEPHEADER_H_
#define LKRMEPHEADER_H_

#include <stdint.h>
#include <vector>
#include <atomic>
#include <boost/noncopyable.hpp>

#include "../exceptions/UnknownCREAMSourceIDFound.h"
#include "../exceptions/BrokenPacketReceivedError.h"
#include "LkrFragment.h"

namespace na62 {
namespace cream {

class LKRMEP: private boost::noncopyable  {
public:
	/**
	 * Reads the data coming from L0 and initializes the corresponding fields
	 */
	LKRMEP(const char * data, const uint16_t& dataLength,
			const char* originalData) throw (BrokenPacketReceivedError,
					UnknownCREAMSourceIDFound);

	/**
	 * Frees the data buffer (orignialData) that was created by the Receiver
	 *
	 * Should only be called by ~LKRMEPFragment() as a LKRMEP may not be deleted until every LKRMEPFragment is processed and deleted.
	 */
	virtual ~LKRMEP();

	void initializeLkrFragments(const char* data, const uint16_t& dataLength)
			throw (UnknownCREAMSourceIDFound, BrokenPacketReceivedError);

	/**
	 * Returns a pointer to the n'th event within this LKRMEP where 0<=n<getFirstEventNum()
	 */
	inline LkrFragment* getEvent(const uint16_t n) {
		/*
		 * n may be bigger than <getNumberOfEvents()> as <deleteEvent()> could have been invoked already
		 */
		return events[n];
	}

	inline uint16_t getNumberOfEvents() const {
		return eventNum;
	}

	/**
	 * Returns true if no more events are remaining (all have been processed and sent/deleted).
	 * So if true this LKRMEP can also be deleted (together with its original UDP packet)
	 *
	 * This method is thread safe
	 */
	inline bool deleteEvent() {
		/*
		 * Decrement eventCount. If we reach 0 we can delete this object as all events have been processed.
		 */
		return --eventNum == 0;
	}

	const char* getEtherFrame() const {
		return etherFrame_;
	}

private:
	std::atomic<int> eventNum;
	// The whole ethernet frame
	const char* etherFrame_;
	// Pointers to the payload of the UDP packet
	std::vector<LkrFragment*> events;
};

} /* namespace cream */
} /* namespace na62 */
#endif /* MEPHEADER_H_ */
