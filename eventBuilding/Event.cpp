/*
 * Event.cpp
 *
 *  Created on: Jan 18, 2012
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#include "Event.h"

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include <glog/logging.h>

#include "../l0/MEPEvent.h"
#include "../l0/Subevent.h"
#include "../LKr/LKREvent.h"

namespace na62 {

Event::Event(uint32_t eventNumber) :
		eventNumber_(eventNumber), numberOfL0Events_(0), numberOfCREAMEvents_(
				0), burstID_(0), triggerTypeWord_(0), timestamp_(0), finetime_(
				0), SOBtimestamp_(0), processingID_(0), nonZSuppressedDataRequestedNum(
				0), L1Processed_(false), L2Accepted_(false), lastEventOfBurst_(
				false) {

	/*
	 * Initialize subevents at the existing sourceIDs as position
	 */
	L0Subevents = new l0::Subevent*[SourceIDManager::NUMBER_OF_L0_DATA_SOURCES];
	for (int i = SourceIDManager::NUMBER_OF_L0_DATA_SOURCES - 1; i >= 0; i--) {
		/*
		 * Initialize subevents[sourceID] with new Subevent(Number of expected Events)
		 */
		L0Subevents[i] = new l0::Subevent(
				SourceIDManager::getExpectedPacksByEventNum(i));
	}

	zSuppressedLKrEventsByCrateCREAMID =
			new cream::LKREvent*[SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT];

	for (int i = SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT
			- 1; i >= 0; i--) {
		zSuppressedLKrEventsByCrateCREAMID[i] = NULL;
	}
}

Event::~Event() {
//	throw NA62Error("An Event-Object should not be deleted! Use Event::destroy instead so that it can be reused by the EventBuilder!");

	for (uint8_t i = 0; i < SourceIDManager::NUMBER_OF_L0_DATA_SOURCES; i++) {
//		L0Subevents[i]->destroy();
		delete L0Subevents[i];
	}
	delete[] L0Subevents;

	for (int ID = 0;
			ID < SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
			ID++) {
		cream::LKREvent* event = zSuppressedLKrEventsByCrateCREAMID[ID];
		if (event != NULL) {
			delete event;
		}
	}
	delete[] zSuppressedLKrEventsByCrateCREAMID;
}

bool Event::addL0Event(l0::MEPEvent* l0Event, uint32_t burstID) {
	// If the new event number does not equal the first one something went terribly wrong!
	if (eventNumber_ != l0Event->getEventNumber()) {
		LOG(ERROR)<<
		"Trying to add MEPEvent with eventNumber " + boost::lexical_cast<std::string>(l0Event->getEventNumber())
		+ " to an Event with eventNumber " + boost::lexical_cast<std::string>(eventNumber_) + ". Will ignore the MEPEvent!";
		delete l0Event;
		return false;
	}

	if (numberOfL0Events_ == 0) {
		lastEventOfBurst_ = l0Event->isLastEventOfBurst();
		setBurstID(burstID);
	} else {
		if (l0Event->isLastEventOfBurst() != lastEventOfBurst_) {
			destroy();
			LOG(ERROR) <<"MEPE Events  'lastEvenOfBurst' flag discords with the flag of the Event with the same eventNumber.";
			return addL0Event(l0Event, burstID);
		}
	}

	if (l0Event->getSourceID() == SourceIDManager::TS_SOURCEID) {
		timestamp_ = l0Event->getTimestamp();
	}

	if (burstID != getBurstID()) {
		/*
		 * Event not build during last burst -> destroy it!
		 */
		LOG(ERROR) <<
		"Overwriting unfinished event from Burst " + boost::lexical_cast<std::string>((int ) getBurstID()) + "! Eventnumber: "
		+ boost::lexical_cast<std::string>((int ) getEventNumber());
		destroy();
		return addL0Event(l0Event, burstID);
	}

	l0::Subevent* subevent = L0Subevents[l0Event->getSourceIDNum()];

	if (subevent->getNumberOfParts() >= SourceIDManager::getExpectedPacksByEventID(l0Event->getSourceID())) {
		/*
		 * Already received enough packets from that sourceID! It seems like this is an old event from the last burst -> destroy it!
		 */
		LOG(ERROR) <<"Event number " << l0Event->getEventNumber() << " already received from source " << ((int) l0Event->getSourceID());
		destroy();
		return addL0Event(l0Event, burstID);
	}

	subevent->addEventPart(l0Event);
	numberOfL0Events_++;

	return numberOfL0Events_ == SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT;
}

bool Event::addLKREvent(cream::LKREvent* lkrEvent) {
	if (!L1Processed_) {
		LOG(ERROR)<<
		"Received LKR data with EventNumber " + boost::lexical_cast<std::string>((int ) lkrEvent->getEventNumber()) + ", crateID "
		+ boost::lexical_cast<std::string>((int ) lkrEvent->getCrateID()) + " and CREAMID "
		+ boost::lexical_cast<std::string>((int ) lkrEvent->getCREAMID())
		+ " before requesting it. Will ignore it as it seems to come from last burst.";
		delete lkrEvent;
		return false;
	}

	if (eventNumber_ != lkrEvent->getEventNumber()) {
		LOG(ERROR) <<
		"Trying to add LKrevent with eventNumber " + boost::lexical_cast<std::string>(lkrEvent->getEventNumber())
		+ " to an Event with eventNumber " + boost::lexical_cast<std::string>(eventNumber_) + ". Will ignore the LKrEvent!";
		delete lkrEvent;
		return false;
	}

	if (nonZSuppressedDataRequestedNum != 0) {
		const uint16_t crateCREAMID = lkrEvent->getCrateCREAMID();
		/*
		 * We were waiting for non zero suppressed data
		 */
		std::map<uint16_t, cream::LKREvent*>::iterator lb = nonSuppressedLKrEventsByCrateCREAMID.lower_bound(crateCREAMID);

		if (lb != nonSuppressedLKrEventsByCrateCREAMID.end() && !(nonSuppressedLKrEventsByCrateCREAMID.key_comp()(crateCREAMID, lb->first))) {
			LOG(ERROR) <<
			"Non zero suppressed LKr event with EventNumber " << (int ) lkrEvent->getEventNumber()
			<< ", crateID " << (int ) lkrEvent->getCrateID() << " and CREAMID " << (int ) lkrEvent->getCREAMID() << " received twice! Will delete the whole event!";

			destroy();
			delete lkrEvent;
			return false;
		} else {
			/*
			 * Event does not yet exist -> add it to the map
			 */
			nonSuppressedLKrEventsByCrateCREAMID.insert(lb, std::map<uint16_t, cream::LKREvent*>::value_type(crateCREAMID, lkrEvent));
		}
		return nonSuppressedLKrEventsByCrateCREAMID.size() == nonZSuppressedDataRequestedNum;
	} else {
		uint16_t localCreamID = SourceIDManager::getLocalCREAMID(lkrEvent->getCrateID(), lkrEvent->getCREAMID());
		/*
		 * This must be a zero suppressed event
		 */
		cream::LKREvent* oldEvent = zSuppressedLKrEventsByCrateCREAMID[localCreamID];

		if (oldEvent != NULL) {
			LOG(ERROR) <<
			"LKr event with EventNumber " + boost::lexical_cast<std::string>((int ) lkrEvent->getEventNumber()) + ", crateID "
			+ boost::lexical_cast<std::string>((int ) lkrEvent->getCrateID()) + " and CREAMID "
			+ boost::lexical_cast<std::string>((int ) lkrEvent->getCREAMID()) + " received twice! Will delete the whole event!";
			destroy();
			delete lkrEvent;
			return false;
		}

		zSuppressedLKrEventsByCrateCREAMID[localCreamID] = lkrEvent;
		numberOfCREAMEvents_++;

		return numberOfCREAMEvents_ == SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
	}
}

void Event::reset() {
	numberOfL0Events_ = 0;
	numberOfCREAMEvents_ = 0;
	burstID_ = 0;
	triggerTypeWord_ = 0;
	timestamp_ = 0;
	finetime_ = 0;
	processingID_ = 0;
	L1Processed_ = false;
	L2Accepted_ = false;
	lastEventOfBurst_ = false;
	nonZSuppressedDataRequestedNum = 0;
}

void Event::destroy() {
	for (uint8_t i = 0; i < SourceIDManager::NUMBER_OF_L0_DATA_SOURCES; i++) {
		L0Subevents[i]->destroy();
	}

	for (int ID = 0;
			ID < SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
			ID++) {
		cream::LKREvent* event = zSuppressedLKrEventsByCrateCREAMID[ID];
		if (event != NULL) {
			delete event;
		}
		zSuppressedLKrEventsByCrateCREAMID[ID] = NULL;
	}

	for (auto pair : nonSuppressedLKrEventsByCrateCREAMID) {
		delete pair.second;
	}
	nonSuppressedLKrEventsByCrateCREAMID.clear();

	reset();
}

void Event::clear() {
	if (numberOfL0Events_ > 0 || numberOfCREAMEvents_ > 0) {
		destroy();
	}
}

} /* namespace na62 */
