/*
 * Event.h
 *
 *  Created on: Jan 18, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef EVENT_H_
#define EVENT_H_

#include <algorithm>
#include <cstdint>
#include <map>
#include <atomic>
#ifdef MEASURE_TIME
#include <boost/timer/timer.hpp>
#endif
#include <boost/noncopyable.hpp>

#include "../LKr/LkrFragment.h"
#include "SourceIDManager.h"

#include <mutex>

//#define MEASURE_TIME

namespace na62 {
namespace cream {
class LkrFragment;

} /* namespace cream */
namespace l0 {

class MEPFragment;

class Subevent;
} /* namespace l0 */
} /* namespace na62 */

namespace na62 {

class Event:   boost::noncopyable{
public:
	Event(uint32_t eventNumber_);
	virtual ~Event();
	/**
	 * Add an Event from a new SourceID.
	 * return <true> if the event was the last missing one <false> if some subevents
	 * are still missing
	 *
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 */
	bool addL0Event(l0::MEPFragment* e, uint32_t burstID);

	/*
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 *
	 * @return [true] if the given LKr event fragment was the last one to complete the event
	 */
	bool addLkrFragment(cream::LkrFragment* fragment);

	/*
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 */
	void destroy();

	/*
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 */
	bool isL1Processed() const {
		return L1Processed_;
	}

	/*
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 */
	bool isL2Accepted() const {
		return L2Accepted_;
	}

	/*
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 */
	void setEventNumber(uint32_t eventNumber) {
		eventNumber_ = eventNumber;
	}

	/**
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 *
	 * Set the trigger type word. If this is 0 the event will be destroyed (after sending the 0x00 tirrger type word to the creams)
	 *
	 * The lower byte is the L0 trigger type word, the upper byte is the one of L1
	 */
	void setL1Processed(const uint16_t L0L1TriggerTypeWord) {
#ifdef MEASURE_TIME
		l1ProcessingTime_ = firstEventPartAddedTime_.elapsed().wall / 1E3
		- l0BuildingTime_;
#endif

		triggerTypeWord_ = L0L1TriggerTypeWord;
		L1Processed_ = true;
	}

	/**
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 *
	 * Set the trigger type word. If this is 0 the event will be destroyed (after sending the 0x00 tirrger type word to the creams)
	 *
	 * The lower byte is the L0 trigger type word, the upper byte is the one of L1
	 */
	void setL2Processed(const uint8_t L2TriggerTypeWord) {
#ifdef MEASURE_TIME
		l2ProcessingTime_ = firstEventPartAddedTime_.elapsed().wall / 1E3
		- l0BuildingTime_;
#endif

		L2Accepted_ = L2TriggerTypeWord > 0;
		// Move the L2 trigger type word to the third byte of triggerTypeWord_
		triggerTypeWord_ |= L2TriggerTypeWord << 16;
	}

	uint32_t getEventNumber() const {
		return eventNumber_;
	}

	uint32_t getTriggerTypeWord() const {
		return triggerTypeWord_;
	}

	uint32_t getTimestamp() const {
		return timestamp_;
	}

	/*
	 * This should be used within the L1 trigger algorithm as the fine time is transmitted within the
	 * L0TP subevent
	 */
	void setFinetime(const uint8_t finetime) {
		finetime_ = finetime;
	}

	/*
	 * This will return an undefined value before the L1 trigger algorithm has executed setFinetime()
	 * Only use it for any L2 trigger algorithm!
	 */
	uint8_t getFinetime() const {
		return finetime_;
	}

	/*
	 * Should be defined by the trigger algorithms L1 or L2
	 */
	void setProcessingID(const uint32_t processingID) {
		processingID_ = processingID;
	}

	/*
	 * This will return an undefined value before any trigger algorithm has executed setProcessingID()
	 */
	uint32_t getProcessingID() const {
		return processingID_;
	}

	/*
	 * Will return the bust number at which this event has been taken
	 */
	uint32_t getBurstID() const {
		return burstID_;
	}

	/*
	 * Can be used for itaration over all subevents like following:
	 * for (int i = Options::Instance()->NUMBER_OF_L0_DATA_SOURCES - 1; i >= 0; i--) {
	 *		Subevent* subevent = event->getL0SubeventBySourceIDNum(i);
	 *		...
	 *	}
	 */
	l0::Subevent* getL0SubeventBySourceIDNum(const uint8_t sourceIDNum) const {
		return L0Subevents[sourceIDNum];
	}

	/*
	 *	See table 50 in the TDR for the source IDs.
	 */
	inline l0::Subevent* getL0SubeventBySourceID(
			const uint8_t&& sourceID) const {
		return L0Subevents[SourceIDManager::SourceIDToNum(std::move(sourceID))];
	}
	inline l0::Subevent* getCEDARSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_CEDAR)];
	}
	inline l0::Subevent* getGTKSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_GTK)];
	}
	inline l0::Subevent* getCHANTISubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_CHANTI)];
	}
	inline l0::Subevent* getLAVSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_LAV)];
	}
	inline l0::Subevent* getSTRAWSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_STRAW)];
	}
	inline l0::Subevent* getCHODSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_CHOD)];
	}
	inline l0::Subevent* getIRCSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_IRC)];
	}
	inline l0::Subevent* getMUVSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_MUV)];
	}
	inline l0::Subevent* getSACSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_SAC)];
	}
	inline l0::Subevent* getL0TPSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_L0TP)];
	}

	/*
	 * Returns a  zero suppressed event fragment sent by the CREAM with the id [CREAMID] in the crate [crateID
	 */
	cream::LkrFragment* getZSuppressedLkrFragment(const uint8_t crateID,
			const uint8_t CREAMID) const {
		return zSuppressedLkrFragmentsByLocalCREAMID[SourceIDManager::getLocalCREAMID(
				crateID, CREAMID)];
	}

	/*
	 * Returns a zero suppressed event fragment sent by the CREAM identified by the given local CREAM ID
	 */
	cream::LkrFragment* getZSuppressedLkrFragment(const uint16_t localCreamID) const {
		return zSuppressedLkrFragmentsByLocalCREAMID[localCreamID];
	}

	uint16_t getNumberOfZSuppressedLkrFragments() const {
		return SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
	}

	cream::LkrFragment* getNonZSuppressedLkrFragment(const uint16_t crateID,
			const uint8_t CREAMID) const {
		return nonSuppressedLkrFragmentsByCrateCREAMID.at(
				cream::LkrFragment::generateCrateCREAMID(crateID, CREAMID));
	}

	/**
	 * Get the received non zero suppressed LKr Event by the crateCREMID (qsee LkrFragment::generateCrateCREAMID)
	 */
	cream::LkrFragment* getNonZSuppressedLkrFragment(
			const uint16_t crateCREAMID) const {
		return nonSuppressedLkrFragmentsByCrateCREAMID.at(crateCREAMID);
	}

	/**
	 * Returns the map containing all received non zero suppressed LKR Events.
	 * The keys are the 24-bit crate-ID and CREAM-ID concatenations (@see LKR_EVENT_RAW_HDR::generateCrateCREAMID)
	 */
	std::map<uint16_t, cream::LkrFragment*> getNonSuppressedLkrFragments() const {
		return nonSuppressedLkrFragmentsByCrateCREAMID;
	}

	/*
	 * This method is mainly used for the transmission to the merger PC but may also be used within any Trigger algorithm
	 */
	bool isLastEventOfBurst() const {
		return lastEventOfBurst_;
	}

	bool isWaitingForNonZSuppressedLKrData() const {
		return nonZSuppressedDataRequestedNum != 0;
	}

	void setNonZSuppressedDataRequestedNum(
			uint16_t nonZSuppressedDataRequestedNum) {
		this->nonZSuppressedDataRequestedNum = nonZSuppressedDataRequestedNum;
	}

	static void setPrintMissingSourceIds(bool doPrint){
		printMissingSourceIDs_ = doPrint;
	}

#ifdef MEASURE_TIME
	/*
	 * Returns the number of wall microseconds since the first event part has been added to this event
	 */
	u_int32_t getTimeSinceFirstMEPReceived() const {
		return firstEventPartAddedTime_.elapsed().wall / 1E3;
	}

	/*
	 * Returns the number of wall microseconds passed between the first and last L0 MEP received
	 */
	u_int32_t getL0BuildingTime() const {
		return l0BuildingTime_;
	}

	/*
	 * Returns the number of wall microseconds passed between the last L0 MEP received and the end of the L1 processing
	 */
	u_int32_t getL1ProcessingTime() const {
		return l1ProcessingTime_;
	}

	/*
	 * Returns the number of wall microseconds passed between the  end of the L1 processing and the last LKr MEP received
	 */
	u_int32_t getL1BuildingTime() const {
		return l1BuildingTime_;
	}

	/*
	 * Returns the number of wall microseconds passed between the  LKr MEP received and the end of the L2 processing
	 */
	u_int32_t getL2ProcessingTime() const {
		return l2ProcessingTime_;
	}
#endif

private:
	void setBurstID(const uint32_t L0ID) {
		burstID_ = L0ID;
	}

	/*
	 * Find the missing sourceIDs
	 */
	std::string getMissingSourceIDs();

	void reset();

	bool storeNonZSuppressedLkrFragemnt(cream::LkrFragment* fragment);

	/*
	 * Don't forget to reset new variables in Event::reset()!
	 */
	uint32_t eventNumber_;
	std::atomic<uint8_t> numberOfL0Events_;
	std::atomic<uint16_t> numberOfCREAMEvents_;

	/*
	 * To be added within L1 trigger process
	 */
	uint32_t burstID_;
	uint32_t triggerTypeWord_;
	uint32_t timestamp_;
	uint8_t finetime_;
	uint32_t SOBtimestamp_;
	uint32_t processingID_;

	l0::Subevent ** L0Subevents;

	uint16_t nonZSuppressedDataRequestedNum;

	/*
	 * zSuppressedLkrFragmentsByLocalCREAMID[SourceIDManager::getLocalCREAMID()] is the cream event fragment of the
	 * corresponding cream/create
	 */
	cream::LkrFragment** zSuppressedLkrFragmentsByLocalCREAMID;
	std::map<uint16_t, cream::LkrFragment*> nonSuppressedLkrFragmentsByCrateCREAMID;

	bool L1Processed_;bool L2Accepted_;

	bool lastEventOfBurst_;

	static bool printMissingSourceIDs_;

#ifdef MEASURE_TIME
	boost::timer::cpu_timer firstEventPartAddedTime_;

	/*
	 * Times in microseconds
	 */
	u_int32_t l0BuildingTime_;
	u_int32_t l1ProcessingTime_;
	u_int32_t l1BuildingTime_;
	u_int32_t l2ProcessingTime_;
#endif
};

} /* namespace na62 */
#endif /* EVENT_H_ */
