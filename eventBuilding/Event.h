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
#include <tbb/spin_mutex.h>

#include "../LKr/LkrFragment.h"
#include "SourceIDManager.h"
#include "../structs/Event.h"

#include <iostream>

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

class Event: boost::noncopyable {
public:
	Event(uint_fast32_t eventNumber_);
	virtual ~Event();
	/**
	 * Add an Event from a new SourceID.
	 * return <true> if the event was the last missing one <false> if some subevents
	 * are still missing
	 *
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 */
	bool addL0Event(l0::MEPFragment* e, uint_fast32_t burstID);

	/*
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 *
	 * @return [true] if the given LKr event fragment was the last one to complete the event
	 */
	bool addLkrFragment(cream::LkrFragment* fragment, uint sourceIP);

	/*
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 */
	void destroy();

	bool isUnfinished() const {
		return unfinished_;
	}

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
	void setEventNumber(uint_fast32_t eventNumber) {
		eventNumber_ = eventNumber;
	}

	/**
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 *
	 * Set the trigger type word. If this is 0 the event will be destroyed (after sending the 0x00 tirrger type word to the creams)
	 *
	 * The lower byte is the L0 trigger type word, the upper byte is the one of L1
	 */
	void setL1Processed(const uint_fast16_t L0L1TriggerTypeWord) {
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
	void setL2Processed(const uint_fast8_t L2TriggerTypeWord) {
#ifdef MEASURE_TIME
		l2ProcessingTime_ = firstEventPartAddedTime_.elapsed().wall / 1E3
		- l0BuildingTime_;
#endif

		L2Accepted_ = L2TriggerTypeWord > 0;
		// Move the L2 trigger type word to the third byte of triggerTypeWord_
		triggerTypeWord_ |= L2TriggerTypeWord << 16;
		unfinished_ = false;
	}

	uint_fast32_t getEventNumber() const {
		return eventNumber_;
	}

	uint_fast32_t getTriggerTypeWord() const {
		return triggerTypeWord_;
	}

	/**
	 * Returns the L0 trigger type word if readTriggerTypeWordAndFineTime has already
	 * been called. The return value is undefined otherwise!
	 */
	uint_fast8_t getL0TriggerTypeWord() const {
		return triggerTypeWord_ & 0xFF;
	}

	/**
	 * Returns the L1 trigger type word if L1 has already been processed.
	 * The return value is undefined otherwise!
	 */
	uint_fast8_t getL1TriggerTypeWord() const {
		return (triggerTypeWord_ >> 8) & 0xFF;
	}

	/**
	 * Returns the L2 trigger type word if L2 has already been processed.
	 * The return value is undefined otherwise!
	 */
	uint_fast32_t getL2TriggerTypeWord() const {
		return (triggerTypeWord_ >> 16) & 0xFF;
	}

	/**
	 * Returns true if the L1 trigger word equals the L1 bypass trigger word meaning
	 * that L1 has not been processed but still the event has been accepted (bypassed)
	 */
	bool isL1Bypassed() const {
		return ((triggerTypeWord_ >> 8) & TRIGGER_L1_BYPASS)
				== TRIGGER_L1_BYPASS;
	}

	/**
	 * Returns true if the L2 trigger word equals the L2 bypass trigger word meaning
	 * that L2 has not been processed but still the event has been accepted (bypassed)
	 */
	bool isL2Bypassed() const {
		return ((triggerTypeWord_ >> 16) & TRIGGER_L2_BYPASS)
				== TRIGGER_L2_BYPASS;
	}

	/**
	 * If set to true during the L1 trigger processing  zero suppressed CREAM data will be requested
	 */
	void setRrequestZeroSuppressedCreamData(bool doRequestZSData) {
		requestZeroSuppressedCreamData_ = doRequestZSData;
	}

	/**
	 * Returns true if zero suppressed CREAM data should be requested
	 */
	bool isRrequestZeroSuppressedCreamData() const {
		return requestZeroSuppressedCreamData_;
	}

	/**
	 * Returns the L0 trigger type word stored in the L0TP data and stores the fineTime. If L0TP is not activated 1 is returned
	 */
	uint_fast8_t readTriggerTypeWordAndFineTime();

	void setTimestamp(const uint_fast32_t time) {
		timestamp_ = time;
	}

	uint_fast32_t getTimestamp() const {
		return timestamp_;
	}

	void setFinetime(const uint_fast8_t finetime) {
		finetime_ = finetime;
	}

	uint_fast8_t getFinetime() const {
		return finetime_;
	}

	void setl0TriggerTypeWord(const uint_fast8_t l0triggertype) {
		triggerTypeWord_ = (triggerTypeWord_ & 0xFFFFFF00) | l0triggertype;
	}
	/*
	 * Should be defined by the trigger algorithms L1 or L2
	 */
	void setProcessingID(const uint_fast32_t processingID) {
		processingID_ = processingID;
	}

	/*
	 * This will return an undefined value before any trigger algorithm has executed setProcessingID()
	 */
	uint_fast32_t getProcessingID() const {
		return processingID_;
	}

	/*
	 * Will return the bust number at which this event has been taken
	 */
	uint_fast32_t getBurstID() const {
		return burstID_;
	}

	/*
	 * Can be used for itaration over all subevents like following:
	 * for (int i = Options::Instance()->NUMBER_OF_L0_DATA_SOURCES - 1; i >= 0; i--) {
	 *		Subevent* subevent = event->getL0SubeventBySourceIDNum(i);
	 *		...
	 *	}
	 */
	l0::Subevent* getL0SubeventBySourceIDNum(
			const uint_fast8_t sourceIDNum) const {
		return L0Subevents[sourceIDNum];
	}

	/*
	 *	See table 50 in the TDR for the source IDs.
	 */
	inline const l0::Subevent* getL0SubeventBySourceID(
			const uint_fast8_t sourceID) const {
		return L0Subevents[SourceIDManager::sourceIDToNum(std::move(sourceID))];
	}
	inline const l0::Subevent* getCEDARSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_CEDAR)];
	}
	inline const l0::Subevent* getGTKSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_GTK)];
	}
	inline const l0::Subevent* getCHANTISubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_CHANTI)];
	}
	inline const l0::Subevent* getLAVSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_LAV)];
	}
	inline const l0::Subevent* getSTRAWSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_STRAW)];
	}
	inline const l0::Subevent* getCHODSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_CHOD)];
	}
	inline const l0::Subevent* getRICHSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_RICH)];
	}
	inline const l0::Subevent* getIRCSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_IRC)];
	}
	inline const l0::Subevent* getMUV3Subevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_MUV3)];
	}
	inline const l0::Subevent* getSACSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_SAC)];
	}
	inline const l0::Subevent* getL0TPSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_L0TP)];
	}
	inline const l0::Subevent* getL1Subevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_L1)];
	}
	inline const l0::Subevent* getL2Subevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_L2)];
	}
	inline const l0::Subevent* getNSTDSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_NSTD)];
	}
	/*
	 * Returns a  zero suppressed event fragment sent by the CREAM with the id [CREAMID] in the crate [crateID
	 */
	inline cream::LkrFragment* getZSuppressedLkrFragment(
			const uint_fast8_t crateID, const uint_fast8_t CREAMID) const {
		return zSuppressedLkrFragmentsByLocalCREAMID[SourceIDManager::getLocalCREAMID(
				crateID, CREAMID)];
	}

	/*
	 * Returns a zero suppressed event fragment sent by the CREAM identified by the given local CREAM ID
	 */
	inline cream::LkrFragment* getZSuppressedLkrFragment(
			const uint_fast16_t localCreamID) const {
		return zSuppressedLkrFragmentsByLocalCREAMID[localCreamID];
	}

	/**
	 * Returns a pointer to an array of LkrFragments with [getNumberOfZSuppressedLkrFragments] elements storing the zero suppressed LKr data
	 */
	inline cream::LkrFragment** getZSuppressedLkrFragments() const {
		return zSuppressedLkrFragmentsByLocalCREAMID;
	}

	inline uint_fast16_t getNumberOfZSuppressedLkrFragments() const {
		return SourceIDManager::NUMBER_OF_EXPECTED_LKR_CREAM_FRAGMENTS;
	}

	/**
	 * Returns the number of MUV1 fragments that are accessible via [getMuv1Fragments] after L2 event building
	 */
	inline uint_fast16_t getNumberOfMuv1Fragments() const {
		return SourceIDManager::MUV1_NUMBER_OF_FRAGMENTS;
	}

	/**
	 * Returns a pointer to an array of LkrFragments with [getNumberOfMuv1Fragments] elements storing the data of MUV1
	 */
	inline cream::LkrFragment** getMuv1Fragments() const {
		/*
		 * MUV1 is the largest crate and therefore the fragments are stored behind the LKr fragments.
		 */
		return &zSuppressedLkrFragmentsByLocalCREAMID[getNumberOfZSuppressedLkrFragments()];
	}

	/**
	 * Returns the number of MUV2 fragments that are accessible via [getMuv2Fragments] after L2 event building
	 */
	inline uint_fast16_t getNumberOfMuv2Fragments() const {
		return SourceIDManager::MUV2_NUMBER_OF_FRAGMENTS;
	}

	/**
	 * Returns a pointer to an array of LkrFragments with [getNumberOfMuv2Fragments] elements storing the data of MUV2
	 */
	inline cream::LkrFragment** getMuv2Fragments() const {
		/*
		 * MUV2 fragments are stored behind the MUV1 fragments.
		 */
		return &zSuppressedLkrFragmentsByLocalCREAMID[getNumberOfZSuppressedLkrFragments()
				+ getNumberOfMuv1Fragments()];
	}

	inline cream::LkrFragment* getNonZSuppressedLkrFragment(
			const uint_fast16_t crateID, const uint_fast8_t CREAMID) const {
		return nonSuppressedLkrFragmentsByCrateCREAMID.at(
				cream::LkrFragment::generateCrateCREAMID(crateID, CREAMID));
	}

	/**
	 * Get the received non zero suppressed LKr Event by the crateCREMID (qsee LkrFragment::generateCrateCREAMID)
	 */
	inline cream::LkrFragment* getNonZSuppressedLkrFragment(
			const uint_fast16_t crateCREAMID) const {
		return nonSuppressedLkrFragmentsByCrateCREAMID.at(crateCREAMID);
	}

	/**
	 * Returns the map containing all received non zero suppressed LKR Events.
	 * The keys are the 24-bit crate-ID and CREAM-ID concatenations (@see LKR_EVENT_RAW_HDR::generateCrateCREAMID)
	 */
	inline std::map<uint_fast16_t, cream::LkrFragment*> getNonSuppressedLkrFragments() const {
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
			uint_fast16_t nonZSuppressedDataRequestedNum) {
		this->nonZSuppressedDataRequestedNum = nonZSuppressedDataRequestedNum;
	}

//	bool isSpecialTriggerEvent() {
//		switch (getL0TriggerTypeWord()) {
//		case TRIGGER_L0_EOB:
//		case TRIGGER_L0_SOB:
//			return true;
//		default:
//			return false;
//		}
//	}

	bool isSpecialTriggerEvent() {
		uint_fast8_t specialTriggerMask = 0x20;
		return ((getL0TriggerTypeWord() & specialTriggerMask) != 0);
	}

	/*
	 * List the received sourceIDs, sourceSubIDs and stats
	 */
//	std::map<uint, std::map<uint, uint>> getReceivedSourceIDsSourceSubIds();
	/*
	 * Find the missing sourceIDs
	 */
	std::map<uint, std::vector<uint>> getMissingSourceIDs();
	std::map<uint, std::vector<uint>> getMissingCreams();

	static uint64_t getMissingEventsBySourceNum(uint sourceNum) {
		return MissingEventsBySourceNum_[sourceNum];
	}

//	static uint64_t getReceivedEventsBySourceNumBySubId(uint sourceNum,
//			uint SubId) {
//		return ReceivedEventsBySourceNumBySubId_[sourceNum][SubId];
//	}
	static uint64_t getNumberOfNonRequestedCreamFragments() {
		return nonRequestsCreamFramesReceived_;
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

	static void initialize(bool printMissingSourceIDs,
	bool writeBrokenCreamInfo);
private:
	void setBurstID(const uint_fast32_t burstID) {
		burstID_ = burstID;
	}

	void reset();

	bool storeNonZSuppressedLkrFragemnt(cream::LkrFragment* fragment);

	/*
	 * Don't forget to reset new variables in Event::reset()!
	 */
	uint_fast32_t eventNumber_;
	std::atomic<uint_fast8_t> numberOfL0Fragments_;
	std::atomic<uint_fast16_t> numberOfCREAMFragments_;

	/*
	 * To be added within L1 trigger process
	 */
	uint_fast32_t burstID_;
	uint_fast32_t triggerTypeWord_;
	uint_fast32_t timestamp_;
	uint_fast8_t finetime_;
	uint_fast32_t SOBtimestamp_;
	uint_fast32_t processingID_;

	bool requestZeroSuppressedCreamData_;

	l0::Subevent ** L0Subevents;

	uint_fast16_t nonZSuppressedDataRequestedNum;

	/*
	 * zSuppressedLkrFragmentsByLocalCREAMID[SourceIDManager::getLocalCREAMID()] is the cream event fragment of the
	 * corresponding cream/create
	 */
	cream::LkrFragment** zSuppressedLkrFragmentsByLocalCREAMID;
	std::map<uint_fast16_t, cream::LkrFragment*> nonSuppressedLkrFragmentsByCrateCREAMID;

	bool L1Processed_;bool L2Accepted_;bool unfinished_;

	bool lastEventOfBurst_;

	tbb::spin_mutex destroyMutex_;
	tbb::spin_mutex unfinishedEventMutex_;

//	static std::atomic<uint64_t>** ReceivedEventsBySourceNumBySubId_;
	static std::atomic<uint64_t>* MissingEventsBySourceNum_;
	static std::atomic<uint64_t> nonRequestsCreamFramesReceived_;
	static bool printMissingSourceIDs_;
	static bool writeBrokenCreamInfo_;
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
