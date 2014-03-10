/*
 * Event.h
 *
 *  Created on: Jan 18, 2012
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#pragma once
#ifndef EVENT_H_
#define EVENT_H_

#include <algorithm>
#include <cstdint>
#include <map>

#include "../LKr/LKREvent.h"
#include "SourceIDManager.h"

namespace na62 {
namespace cream {
class LKREvent;

} /* namespace cream */
namespace l0 {

class MEPEvent;

class Subevent;
} /* namespace l0 */
} /* namespace na62 */

namespace na62 {

class Event {
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
	bool addL0Event(l0::MEPEvent* e, uint32_t burstID);

	/*
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 */
	bool addLKREvent(cream::LKREvent* e);

	/*
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 */
	void destroy();

	/*
	 * DO NOT USE THIS METHOD IF YOUR ARE IMPLEMENTING TRIGGER ALGORITHMS
	 */
	void clear();

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

	/**
	 * Set the trigger type word. If this is 0 the event will be destroyed (after sending the 0x00 tirrger type word to the creams)
	 *
	 * The lower byte is the L0 trigger type word, the upper byte is the one of L1
	 */
	void setL1Processed(const uint16_t L0L1TriggerTypeWord) {
		triggerTypeWord_ = L0L1TriggerTypeWord;
		L1Processed_ = true;
	}

	/**
	 * Set the trigger type word. If this is 0 the event will be destroyed (after sending the 0x00 tirrger type word to the creams)
	 *
	 * The lower byte is the L0 trigger type word, the upper byte is the one of L1
	 */
	void setL2Processed(const uint8_t L2TriggerTypeWord) {
		L2Accepted_ = L2TriggerTypeWord > 0;
		// Move the L2 trigger type word to the third byte of triggerTypeWord_
		triggerTypeWord_ |= L2TriggerTypeWord << 16;
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
	l0::Subevent* getL0SubeventBySourceID(const uint8_t&& sourceID) const {
		return L0Subevents[SourceIDManager::SourceIDToNum(std::move(sourceID))];
	}
	l0::Subevent* getCEDARSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_CEDAR)];
	}
	l0::Subevent* getGTKSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_GTK)];
	}
	l0::Subevent* getCHANTISubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_CHANTI)];
	}
	l0::Subevent* getLAVSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_LAV)];
	}
	l0::Subevent* getSTRAWSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_STRAW)];
	}
	l0::Subevent* getCHODSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_CHOD)];
	}
	l0::Subevent* getIRCSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_IRC)];
	}
	l0::Subevent* getMUVSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_MUV)];
	}
	l0::Subevent* getSACSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_SAC)];
	}
	l0::Subevent* getL0TPSubevent() const {
		return L0Subevents[SourceIDManager::SourceIDToNum(SOURCE_ID_L0TP)];
	}

	/*
	 * You may access this method only within any TriggerProcessor instance
	 */
	cream::LKREvent* getZSuppressedLKrEvent(const uint8_t crateID,
			const uint8_t CREAMID) const {
		return zSuppressedLKrEventsByCrateCREAMID[SourceIDManager::getLocalCREAMID(
				crateID, CREAMID)];
	}

	/*
	 * You may access this method only within any TriggerProcessor instance
	 */
	cream::LKREvent* getZSuppressedLKrEvent(const uint16_t localCreamID) const {
		return zSuppressedLKrEventsByCrateCREAMID[localCreamID];
	}

	uint16_t getNumberOfZSuppressedLKrEvents() const {
		return SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
	}

	cream::LKREvent* getNonZSuppressedLKrEvent(const uint16_t crateID,
			const uint8_t CREAMID) const {
		return nonSuppressedLKrEventsByCrateCREAMID.at(
				cream::LKREvent::generateCrateCREAMID(crateID, CREAMID));
	}

	/**
	 * Get the received non zero suppressed LKr Event by the crateCREMID (qsee LKREvent::generateCrateCREAMID)
	 */
	cream::LKREvent* getNonZSuppressedLKrEvent(
			const uint16_t crateCREAMID) const {
		return nonSuppressedLKrEventsByCrateCREAMID.at(crateCREAMID);
	}

	/**
	 * Returns the map containing all received non zero suppressed LKR Events.
	 * The keys are the 24-bit crate-ID and CREAM-ID concatenations (@see LKR_EVENT_RAW_HDR::generateCrateCREAMID)
	 */
	std::map<uint16_t, cream::LKREvent*> getNonSuppressedLKrEvents() const {
		return nonSuppressedLKrEventsByCrateCREAMID;
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

private:
	void setBurstID(const uint32_t L0ID) {
		burstID_ = L0ID;
	}

	/*
	 * Don't forget to reset new variables in Event::reset()!
	 */
	uint32_t eventNumber_;
	uint8_t numberOfL0Events_;
	uint16_t numberOfCREAMEvents_;

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
	 * lkrEventsByCreamIDByCrate[crate][cream] is the Event coming from CREAM number <cream> within the crate number <crate>
	 */
	cream::LKREvent** zSuppressedLKrEventsByCrateCREAMID;
	std::map<uint16_t, cream::LKREvent*> nonSuppressedLKrEventsByCrateCREAMID;

	bool L1Processed_;
	bool L2Accepted_;

	bool lastEventOfBurst_;
	void reset();
};

} /* namespace na62 */
#endif /* EVENT_H_ */
