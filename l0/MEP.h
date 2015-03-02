/*
 * Header.h
 *
 *  Created on: Sep 21, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef MEPHEADER_H_
#define MEPHEADER_H_

#include <atomic>
#include <cstdint>
#include <boost/noncopyable.hpp>

#include "../eventBuilding/SourceIDManager.h"
#include "../exceptions/BrokenPacketReceivedError.h"
#include "../exceptions/UnknownSourceIDFound.h"

namespace na62 {
class BrokenPacketReceivedError;
class UnknownSourceIDFound;
} /* namespace na62 */

namespace na62 {
namespace l0 {
class MEPFragment;
} /* namespace l0 */
} /* namespace na62 */

namespace na62 {
namespace l0 {

/**
 * Defines the structure of a L0 MEP header as defined in table 2 in NA62-11-02.
 */
struct MEP_HDR {
	// Number of L0 triggers since start of burst
	uint32_t firstEventNum :24;
	uint8_t sourceID;

	uint16_t mepLength; //  Total length of the MEP in bytes including the header
	uint8_t eventCount;
	uint8_t sourceSubID;
}__attribute__ ((__packed__));

class MEP: private boost::noncopyable {
public:
	/**
	 * Reads the data coming from L0 and initializes the corresponding fields
	 */
	MEP(const char *data, const uint16_t & dataLength, const char *originalData)
			throw (BrokenPacketReceivedError, UnknownSourceIDFound);

	/**
	 * Frees the data buffer (orignialData) that was created by the Receiver
	 *
	 * Should only be called by ~MEPFragment() as a MEP may not be deleted until every MEPFragment is processed and deleted.
	 */
	virtual ~MEP();

	void initializeMEPFragments(const char* data, const uint16_t& dataLength)
			throw (BrokenPacketReceivedError);

	/**
	 * Returns a pointer to the n'th event within this MEP where 0<=n<getFirstEventNum()
	 */
	inline MEPFragment* getFragment(const uint16_t n) {
		/*
		 * n may be bigger than <getNumberOfEvents()> as <deleteEvent()> could have been invoked already
		 */
		return fragments[n];
	}

	inline uint8_t getSourceID() const {
		return rawData->sourceID;
	}

	/**
	 * This method is used to "plug holes" in the data source IDs. So if you have 3 sources being not in a row like {2, 5, 7}
	 * you would probably want to have an array with three entries, one for each source. For this you need a relation like 2->0, 5->1, 7->2.
	 * This is done by this method!
	 */
	inline uint8_t getSourceIDNum() const {
		return SourceIDManager::sourceIDToNum(rawData->sourceID);
	}

	inline uint32_t getFirstEventNum() const {
		return rawData->firstEventNum;
	}

	inline uint16_t getNumberOfEvents() const {
		return rawData->eventCount;
	}

	/**
	 * Total length of the MEP in bytes including the header
	 */
	inline uint16_t getLength() const {
		return rawData->mepLength;
	}

	inline uint8_t getSourceSubID() const {
		return rawData->sourceSubID;
	}

	inline const char* getUDPPack() const {
		return etherFrame_;
	}

	/**
	 * Returns true if no more events are remaining (all have been processed and sent/deleted).
	 * So if true this MEP can also be deleted (together with its original UDP packet)
	 */
	inline bool deleteEvent() {
		/*
		 * Decrement eventCount. If we reach 0 we can delete this object as all events have been processed.
		 */

		// TODO: Do we need to lock here? Probably not locking is that much faster that it's worth implementing a garbage collector?!
		return --eventCount_ == 0;
	}

	const char* getRawData() const {
		return etherFrame_;
	}

//	bool verifyChecksums();

private:
	std::atomic<int> eventCount_;

	// The whole Ethernet frame
	const char* etherFrame_;

	// Pointer to the payload of the UDP packet
	struct MEP_HDR * rawData;

	MEPFragment **fragments;

	bool checkSumsVarified_;
};

} /* namespace l2 */
} /* namespace na62 */
#endif /* MEPHEADER_H_ */
