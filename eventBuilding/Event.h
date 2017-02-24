/*
 * Event.h
 *
 *  Created on: Jan 18, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef EVENT_H_
#define EVENT_H_

#define MEASURE_TIME

#include <algorithm>
#include <cstdint>
#include <map>
#include <atomic>
#include <array>
#ifdef MEASURE_TIME
#include <boost/timer/timer.hpp>
#endif
#include <boost/noncopyable.hpp>
#include <tbb/spin_mutex.h>
#include "SourceIDManager.h"
#include "../structs/Event.h"
#include "../options/Logging.h"
#include "../l1/L1InfoToStorage.h"

#include <iostream>

namespace na62 {
namespace l1 {
class MEPFragment;
class Subevent;
} /* namespace l1 */

namespace l0 {
class MEPFragment;
class Subevent;
} /* namespace l0 */
} /* namespace na62 */

namespace na62 {

class Event: boost::noncopyable {
public:
	Event(uint_fast32_t eventNumber_);
	Event(EVENT_HDR* serializedEvent, bool onlyL0);
	virtual ~Event();
	/**
	 * Add an Event from a new SourceID.
	 * return <true> if the event was the last missing one <false> if some subevents
	 * are still missing
	 *
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 */
	bool addL0Fragment(l0::MEPFragment* e, uint_fast32_t burstID);

	/*
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 *
	 * @return [true] if the given L1 event fragment was the last one to complete the event
	 */
	bool addL1Fragment(l1::MEPFragment* fragment);

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

	/**
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 *
	 * Set the trigger type word. If this is 0 the event will be destroyed (after sending the 0x00 tirrger type word to the creams)
	 *
	 * The lower byte is the L0 trigger type word, the upper byte is the one of L1
	 */
	void setL1Processed(const uint_fast16_t L0L1TriggerTypeWord) {
#ifdef MEASURE_TIME
		l1ProcessingTime_ = firstEventPartAddedTime_.elapsed().wall / 1E3 - l0BuildingTime_;
		//LOG_INFO("*******************l1ProcessingTime_ " << l1ProcessingTime_);
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
		l2ProcessingTime_ = firstEventPartAddedTime_.elapsed().wall / 1E3 - (l1BuildingTime_ + l1ProcessingTime_ + l0BuildingTime_);
//		LOG_INFO("*******************l2ProcessingTime_ " << l2ProcessingTime_);
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
	 * Returns the global L1 trigger type word if L1 has already been processed.
	 * The return value is undefined otherwise!
	 */
	uint_fast8_t getL1TriggerTypeWord() const {
		return (triggerTypeWord_ >> 8) & 0xFF;
	}

	/**
	 * Returns the global L2 trigger type word if L2 has already been processed.
	 * The return value is undefined otherwise!
	 */
	uint_fast32_t getL2TriggerTypeWord() const {
		return (triggerTypeWord_ >> 16) & 0xFF;
	}

	/**
	 * Returns true if the L1 trigger word equals the L1 bypass trigger word meaning
	 * that L1 has not been processed but still the event has been accepted (exclusively bypassed)
	 */
	bool isL1Bypassed() const {
		return ((triggerTypeWord_ >> 8) & TRIGGER_L1_BYPASS) == TRIGGER_L1_BYPASS;
	}

	/**
	 * Returns true if the L2 trigger word equals the L2 bypass trigger word meaning
	 * that L2 has not been processed but still the event has been accepted (exclusively bypassed)
	 */
	bool isL2Bypassed() const {
		return ((triggerTypeWord_ >> 16) & TRIGGER_L2_BYPASS) == TRIGGER_L2_BYPASS;
	}

	/**
	 * Set the L1 trigger word (for a given mask id) after the L1 compute
	 */
	void setL1TriggerWord(uint l0MaskId, uint_fast8_t triggerWord) {
		l1TriggerWords_[l0MaskId] = triggerWord;
	}

	/**
	 * Return the L1 trigger word for a given mask id
	 */
	uint_fast8_t getL1TriggerWord(uint l0MaskId) const {
		return l1TriggerWords_[l0MaskId];
	}
	/**
	 * Return the L1 trigger word for a given mask id
	 */
	std::array<uint_fast8_t, 16> getL1TriggerWords() const {
		return l1TriggerWords_;
	}

	/**
	 * Set the L2 trigger word (for a given mask id) after the L2 compute
	 */
	void setL2TriggerWord(uint l0MaskId, uint_fast8_t triggerWord) {
		l2TriggerWords_[l0MaskId] = triggerWord;
	}

	/**
	 * Return the L2 trigger word for a given mask id
	 */
	uint_fast8_t getL2TriggerWord(uint l0MaskId) const {
		return l2TriggerWords_[l0MaskId];
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
	 * Returns the L0 trigger type word and trigger flags stored in the L0TP data and stores the fineTime. If L0TP is not activated 1 is returned
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

	void setTriggerDataType(const uint_fast8_t triggerDataType) {
		triggerDataType_ = triggerDataType;
	}

	uint_fast8_t getTriggerDataType() const {
		return triggerDataType_;
	}

	void setTriggerFlags(const uint_fast16_t triggerFlags) {
		triggerFlags_ = triggerFlags;
	}

	uint_fast8_t getTriggerFlags() const {
		return triggerFlags_;
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
	 * Can be used for iteration over all L0 subevents like following:
	 * for (int i = Options::Instance()->NUMBER_OF_L0_DATA_SOURCES - 1; i >= 0; i--) {
	 *		Subevent* subevent = event->getL0SubeventBySourceIDNum(i);
	 *		...
	 *	}
	 */
	l0::Subevent* getL0SubeventBySourceIDNum(const uint_fast8_t sourceIDNum) const {
		return L0Subevents[sourceIDNum];
	}

	/*
	 *	See table 50 in the TDR for the source IDs.
	 */
	inline const l0::Subevent* getL0SubeventBySourceID(const uint_fast8_t sourceID) const {
		return L0Subevents[SourceIDManager::sourceIDToNum(std::move(sourceID))];
	}

	/*
	 * Can be used for iteration over all L1 subevents like following:
	 * for (int i = Options::Instance()->NUMBER_OF_L0_DATA_SOURCES - 1; i >= 0; i--) {
	 *		Subevent* subevent = event->getL0SubeventBySourceIDNum(i);
	 *		...
	 *	}
	 */
	l1::Subevent* getL1SubeventBySourceIDNum(const uint_fast8_t sourceIDNum) const {
		return L1Subevents[sourceIDNum];
	}

	/*
	 *	See table 50 in the TDR for the source IDs.
	 */
	inline const l1::Subevent* getL1SubeventBySourceID(const uint_fast8_t sourceID) const {
		return L1Subevents[SourceIDManager::l1SourceIDToNum(std::move(sourceID))];
	}

	inline const l0::Subevent* getCEDARSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_CEDAR)];
	}
	inline const l0::Subevent* getL0GTKSubevent() const {
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
	inline const l0::Subevent* getNewCHODSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_NEWCHOD)];
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
	inline const l0::Subevent* getL1ResultSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_L1)];
	}
	inline const l0::Subevent* getL2ResultSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_L2)];
	}
	inline const l0::Subevent* getNSTDSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_NSTD)];
	}
	inline const l0::Subevent* getHASCSubevent() const {
		return L0Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_HASC)];
	}

// L1 dets
	inline const l1::Subevent* getL1GTKSubevent() const {
		return L1Subevents[SourceIDManager::l1SourceIDToNum(SOURCE_ID_GTK)];
	}
	inline const l1::Subevent* getLKrSubevent() const {
		return L1Subevents[SourceIDManager::l1SourceIDToNum(SOURCE_ID_LKr)];
	}

	inline l1::Subevent* getMuv1Subevent() const {
		return L1Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_MUV1)];
	}

	inline l1::Subevent* getMuv2Subevent() const {
		return L1Subevents[SourceIDManager::sourceIDToNum(SOURCE_ID_MUV2)];
	}

	/**
	 * Get the received non zero suppressed LKr Event by the crateCREAMID
	 */
	inline l1::MEPFragment* getNonZSuppressedLkrFragment(const uint_fast16_t crateCREAMID) const {
		return nonSuppressedLkrFragmentsByCrateCREAMID.at(crateCREAMID);
	}

	/**
	 * Returns the map containing all received non zero suppressed LKR Events.
	 * The keys are the 16-bit crate-ID and CREAM-ID concatenations
	 */
	inline std::map<uint_fast16_t, l1::MEPFragment*> getNonSuppressedLkrFragments() const {
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

	void setNonZSuppressedDataRequestedNum(uint_fast16_t nonZSuppressedDataRequestedNum) {
		this->nonZSuppressedDataRequestedNum = nonZSuppressedDataRequestedNum;
	}

	bool isPhysicsTriggerEvent() {
		return (getTriggerDataType() & TRIGGER_L0_PHYSICS_TYPE);
	}
	bool isPeriodicTriggerEvent() {
		return (getTriggerDataType() & TRIGGER_L0_PERIODICS_TYPE);
	}
	bool isControlTriggerEvent() {
		return (getTriggerDataType() & TRIGGER_L0_CONTROL_TYPE);
	}

//	bool isSpecialTriggerEvent() {
//		uint_fast8_t specialTriggerMask = 0x20;
//		return ((getL0TriggerTypeWord() & specialTriggerMask) != 0);
//	}
	bool isSpecialTriggerEvent() {
		switch (getL0TriggerTypeWord()) {
		case TRIGGER_L0_SOB: 			  //0x22
		case TRIGGER_L0_EOB: 			  //0x23
		case TRIGGER_L0_PEDESTAL_LKR:     //0x2d
		case TRIGGER_L0_CALIBRATION1_LKR: //0x30
		case TRIGGER_L0_CALIBRATION2_LKR: //0x31
		case TRIGGER_L0_CALIBRATION3_LKR: //0x32
		case TRIGGER_L0_CALIBRATION4_LKR: //0x33
			return true;
		default:
			return false;
		}
	}
	bool isPulserGTKTriggerEvent() {
		return (getL0TriggerTypeWord() == TRIGGER_L0_PULSER_GTK);
	}
	/*
	 * Find the missing sourceIDs
	 */
	void updateMissingEventsStats();
	static uint_fast64_t getMissingL0EventsBySourceNum(const uint_fast16_t sourceNum) {
		return MissingEventsBySourceNum_[sourceNum];
	}
	static uint_fast64_t getMissingL1EventsBySourceNum(const uint_fast16_t sourceNum) {
		return MissingL1EventsBySourceNum_[sourceNum];
	}

	std::map<uint, std::vector<uint>> getFilledL0SourceIDs();
	std::map<uint, std::vector<uint>> getFilledL1SourceIDs();

	static uint64_t getNumberOfNonRequestedL1Fragments() {
		return nonRequestsL1FramesReceived_;
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

	uint_fast16_t getL0CallCounter() const {
		return l0CallCounter_;
	}
	void setL1Requested() {
		isL1Requested_ = true;
	}
	bool isL1Requested() {
		return isL1Requested_;
	}

	uint_fast16_t getL1CallCounter() const {
		return l1CallCounter_;
	}

#endif

	static void initialize(bool printCompletedSourceIDs);

private:
	void setBurstID(const uint_fast32_t burstID) {
		burstID_ = burstID;
	}

	void setEventNumber(uint_fast32_t eventNumber) {
		eventNumber_ = eventNumber;
	}

	void reset();

	void resetTriggerWords();

	bool storeNonZSuppressedLkrFragemnt(l1::MEPFragment* fragment);

	/*
	 * Don't forget to reset new variables in Event::reset()!
	 */
	std::atomic<uint_fast32_t> eventNumber_;
	std::atomic<uint_fast8_t> numberOfL0Fragments_;
	std::atomic<uint_fast16_t> numberOfMEPFragments_;

	/*
	 * To be added within L1 trigger process
	 */
	std::atomic<uint_fast32_t> burstID_;
	std::atomic<uint_fast32_t> triggerTypeWord_;
	std::atomic<uint_fast8_t> triggerDataType_;
	std::atomic<uint_fast16_t> triggerFlags_;
	std::atomic<uint_fast32_t> timestamp_;
	std::atomic<uint_fast8_t> finetime_;
	std::atomic<uint_fast32_t> SOBtimestamp_;
	std::atomic<uint_fast32_t> processingID_;

	std::atomic<bool> requestZeroSuppressedCreamData_;

	l0::Subevent ** L0Subevents;
	l1::Subevent ** L1Subevents;

	std::atomic<uint_fast16_t> nonZSuppressedDataRequestedNum;

	/*
	 * zSuppressedLkrFragmentsByLocalCREAMID[SourceIDManager::getLocalCREAMID()] is the cream event fragment of the
	 * corresponding cream/create
	 */

	std::map<uint_fast16_t, l1::MEPFragment*> nonSuppressedLkrFragmentsByCrateCREAMID;

	std::atomic<bool> L1Processed_;
	std::array<uint_fast8_t, 16> l1TriggerWords_;
	std::array<uint_fast8_t, 16> l2TriggerWords_;

	std::atomic<bool> isL1Requested_;
	std::atomic<uint_fast16_t> l0CallCounter_;
	std::atomic<uint_fast16_t> l1CallCounter_;

	std::atomic<bool> L2Accepted_;
	std::atomic<bool> unfinished_;

	std::atomic<bool> lastEventOfBurst_;

	tbb::spin_mutex destroyMutex_;
	tbb::spin_mutex unfinishedEventMutex_;

	static std::atomic<uint64_t>* MissingEventsBySourceNum_;
	static std::atomic<uint64_t>* MissingL1EventsBySourceNum_;

	static std::atomic<uint64_t> nonRequestsL1FramesReceived_;
	static bool printCompletedSourceIDs_;

#ifdef MEASURE_TIME
	boost::timer::cpu_timer firstEventPartAddedTime_;

	/*
	 * Times in microseconds
	 */
	std::atomic<uint_fast32_t> l0BuildingTime_;
	std::atomic<uint_fast32_t> l1ProcessingTime_;
	std::atomic<uint_fast32_t> l1BuildingTime_;
	std::atomic<uint_fast32_t> l2ProcessingTime_;
#endif
};

} /* namespace na62 */
#endif /* EVENT_H_ */
