/*
 * Event.cpp
 *
 *  Created on: Jan 18, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "Event.h"

#include <boost/lexical_cast.hpp>
#include <cstdbool>
#include <sstream>

#include "../exceptions/NA62Error.h"
#include "EventPool.h"

#ifdef USE_GLOG
#include <glog/logging.h>
#endif
#include <sys/types.h>
#include <iostream>
#include <string>
#include <utility>

#include "../utils/DataDumper.h"
#include "../l0/MEPFragment.h"
#include "../l0/Subevent.h"

namespace na62 {
bool Event::printMissingSourceIDs_ = true;

Event::Event(uint32_t eventNumber) :
		eventNumber_(eventNumber), numberOfL0Events_(0), numberOfCREAMEvents_(
				0), burstID_(0), triggerTypeWord_(0), timestamp_(0), finetime_(
				0), SOBtimestamp_(0), processingID_(0), nonZSuppressedDataRequestedNum(
				0), L1Processed_(false), L2Accepted_(false), lastEventOfBurst_(
		false)
#ifdef MEASURE_TIME
, l0BuildingTime_(0), l1ProcessingTime_(0), l1BuildingTime_(0), l2ProcessingTime_(
		0)
#endif
{
#ifdef MEASURE_TIME
	firstEventPartAddedTime_.stop(); //We'll start the first time addL0Event is called
#endif
	/*
	 * Initialize subevents at the existing sourceIDs as position
	 */
	L0Subevents = new l0::Subevent*[SourceIDManager::NUMBER_OF_L0_DATA_SOURCES];
	for (int i = SourceIDManager::NUMBER_OF_L0_DATA_SOURCES - 1; i >= 0; i--) {
		/*
		 * Initialize subevents[sourceID] with new Subevent(Number of expected Events)
		 */
		L0Subevents[i] = new l0::Subevent(
				SourceIDManager::getExpectedPacksBySourceNum(i));
	}

	zSuppressedLkrFragmentsByLocalCREAMID =
			new cream::LkrFragment*[SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT];

	for (int i = SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT
			- 1; i >= 0; i--) {
		zSuppressedLkrFragmentsByLocalCREAMID[i] = nullptr;
	}
}

Event::~Event() {
	throw NA62Error(
			"An Event-Object should not be deleted! Use EventPool::FreeEvent instead so that it can be reused by the EventBuilder!");

//	for (uint8_t i = 0; i < SourceIDManager::NUMBER_OF_L0_DATA_SOURCES; i++) {
////		L0Subevents[i]->destroy();
//		delete L0Subevents[i];
//	}
//	delete[] L0Subevents;
//
//	for (int ID = 0;
//			ID < SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
//			ID++) {
//		cream::LkrFragment* event = zSuppressedLkrFragmentsByLocalCREAMID[ID];
//		if (event != NULL) {
//			delete event;
//		}
//	}
//	delete[] zSuppressedLkrFragmentsByLocalCREAMID;
}

/**
 * Process data coming from the TEL boards
 */
bool Event::addL0Event(l0::MEPFragment* l0Event, uint32_t burstID) {
#ifdef MEASURE_TIME
	if (firstEventPartAddedTime_.is_stopped()) {
		firstEventPartAddedTime_.start();
	}
#endif
//	if (eventNumber_ != l0Event->getEventNumber()) {
//		#ifdef USE_GLOG
//			#ifdef USE_GLOG
//		LOG(INFO)
//#else
//			std::cerr
//#endif
//		<<"Trying to add MEPFragment with eventNumber " + boost::lexical_cast<std::string>(l0Event->getEventNumber())
//		+ " to an Event with eventNumber " + boost::lexical_cast<std::string>(eventNumber_) + ". Will ignore the MEPFragment!";
//		delete l0Event;
//		return false;
//	}

	if (numberOfL0Events_ == 0) {
		lastEventOfBurst_ = l0Event->isLastEventOfBurst();
		setBurstID(burstID);
	} else {
		if (l0Event->isLastEventOfBurst() != lastEventOfBurst_) {
			EventPool::FreeEvent(this);
#ifdef USE_GLOG
			LOG(INFO)
#else
			std::cerr
#endif

<<			"MEPE Events  'lastEvenOfBurst' flag discords with the flag of the Event with the same eventNumber.";
			return addL0Event(l0Event, burstID);
		}

		if (burstID != getBurstID()) {
			/*
			 * Event not build during last burst -> destroy it!
			 */
			if (printMissingSourceIDs_) {
#ifdef USE_GLOG
				LOG(INFO)
#else
				std::cerr
#endif
<<				"Overwriting unfinished event from Burst " << (int) getBurstID()
				<< "! Eventnumber " << (int) getEventNumber()
				<< " misses data from sourceIDs " << getMissingSourceIDs()
#ifndef USE_GLOG
				<< std::endl
#endif
				;
			}
			EventPool::FreeEvent(this);
			return addL0Event(l0Event, burstID);
		}
	}

	/*
	 * Store the global event timestamp if the source ID is the TS_SOURCEID
	 */
	if (l0Event->getSourceID() == SourceIDManager::TS_SOURCEID) {
		timestamp_ = l0Event->getTimestamp();
	}

	l0::Subevent* subevent = L0Subevents[l0Event->getSourceIDNum()];

	if (subevent->getNumberOfFragments()
			>= SourceIDManager::getExpectedPacksBySourceID(
					l0Event->getSourceID())) {
		/*
		 * Already received enough packets from that sourceID! It seems like this is an old event from the last burst -> destroy it!
		 */
		if(printMissingSourceIDs_) {
#ifdef USE_GLOG
			LOG(ERROR)
#else
			std::cerr
#endif
			<< "Event number " << l0Event->getEventNumber()
			<< " already received from source "
			<< ((int) l0Event->getSourceID())
			<< "\nData from following sourceIDs is missing: "
			<< getMissingSourceIDs()
#ifndef USE_GLOG
			<< std::endl
#endif
			;
		}

		EventPool::FreeEvent(this);
		return addL0Event(l0Event, burstID);
	}

	subevent->addFragment(l0Event);

	int currentValue = numberOfL0Events_.fetch_add(1/*,
			 std::memory_order_relaxed*/) + 1;

#ifdef MEASURE_TIME
	if (currentValue
			== SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT) {
		l0BuildingTime_ = firstEventPartAddedTime_.elapsed().wall / 1E3;

		return true;
	}
	return false;
#else
	return currentValue
	== SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT;
#endif
}

bool Event::storeNonZSuppressedLkrFragemnt(cream::LkrFragment* fragment) {
	/*
	 * The received LkrFragment should be nonZSuppressed data
	 */

	const uint16_t crateCREAMID = fragment->getCrateCREAMID();
	/*
	 * We were waiting for non zero suppressed data
	 */
	std::map<uint16_t, cream::LkrFragment*>::iterator lb =
			nonSuppressedLkrFragmentsByCrateCREAMID.lower_bound(crateCREAMID);

	if (lb != nonSuppressedLkrFragmentsByCrateCREAMID.end()
			&& !(nonSuppressedLkrFragmentsByCrateCREAMID.key_comp()(
					crateCREAMID, lb->first))) {
#ifdef USE_GLOG
		LOG(INFO)
#else
		std::cerr
#endif

<<		"Non zero suppressed LKr event with EventNumber "
		<< (int) fragment->getEventNumber() << ", crateID "
		<< (int) fragment->getCrateID() << " and CREAMID "
		<< (int) fragment->getCREAMID()
		<< " received twice! Will delete the whole event!";

		EventPool::FreeEvent(this);
		delete fragment;
		return false;
	} else {
		/*
		 * Event does not yet exist -> add it to the map
		 */
		nonSuppressedLkrFragmentsByCrateCREAMID.insert(lb,
				std::map<uint16_t, cream::LkrFragment*>::value_type(crateCREAMID,
						fragment));
	}
	// TODO: this must be synchronized
	return nonSuppressedLkrFragmentsByCrateCREAMID.size()
			== nonZSuppressedDataRequestedNum;
}

/**
 * Process data coming from the CREAMs
 */
bool Event::addLkrFragment(cream::LkrFragment* fragment) {
//	if (!L1Processed_) {
//#ifdef USE_GLOG
//		LOG(ERROR)
//#else
//		std::cerr
//#endif
//		<< "Received LKR data with EventNumber "
//				<< (int) fragment->getEventNumber() + ", crateID "
//				<< (int) fragment->getCrateID() + " and CREAMID "
//				<< (int) fragment->getCREAMID()
//				<< " before requesting it. Will ignore it as it seems to come from last burst ( current burst is "
//				<< getBurstID() << ")"
//#ifndef USE_GLOG
//				<< std::endl
//#endif
//				;
//
//		delete fragment;
//		return false;
//	}
//
//	if (eventNumber_ != fragment->getEventNumber()) {
//#ifdef USE_GLOG
//		LOG(ERROR)
//#else
//		std::cerr
//#endif
//				<< "Trying to add LkrFragment with eventNumber "
//						+ boost::lexical_cast<std::string>(
//								fragment->getEventNumber())
//						+ " to an Event with eventNumber "
//						+ boost::lexical_cast<std::string>(eventNumber_)
//						+ ". Will ignore the LkrFragment!"
//#ifndef USE_GLOG
//				<< std::endl
//#endif
//				;
//		delete fragment;
//		return false;
//	}

	if (nonZSuppressedDataRequestedNum != 0) {
		return storeNonZSuppressedLkrFragemnt(fragment);
	} else {
		/*
		 * ZSuppressed data received
		 */
		uint16_t localCreamID = SourceIDManager::getLocalCREAMID(
				fragment->getCrateID(), fragment->getCREAMID());
		/*
		 * This must be a zero suppressed event
		 */
//		cream::LkrFragment* oldEvent =
//				zSuppressedLkrFragmentsByLocalCREAMID[localCreamID];
//
//		if (oldEvent != NULL) {
//#ifdef USE_GLOG
//			LOG(INFO)
//#else
//			std::cerr
//#endif
//
//					<< "LKr event with EventNumber "
//							+ boost::lexical_cast<std::string>(
//									(int) fragment->getEventNumber())
//							+ ", crateID "
//							+ boost::lexical_cast<std::string>(
//									(int) fragment->getCrateID())
//							+ " and CREAMID "
//							+ boost::lexical_cast<std::string>(
//									(int) fragment->getCREAMID())
//							+ " received twice! Will delete the whole event!";
//			EventPool::FreeEvent(this);
//			delete fragment;
//			return false;
//		}
		zSuppressedLkrFragmentsByLocalCREAMID[localCreamID] = fragment;

		int currentValue = numberOfCREAMEvents_.fetch_add(1/*,
		 std::memory_order_relaxed*/) + 1;

#ifdef MEASURE_TIME
		if (currentValue == SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT) {
			l1BuildingTime_ = firstEventPartAddedTime_.elapsed().wall/ 1E3-l1ProcessingTime_;
			return true;
		}
		return false;
#else

		return currentValue
				== SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
#endif
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
#ifdef MEASURE_TIME
	firstEventPartAddedTime_.stop();
#endif

	for (uint8_t i = 0; i < SourceIDManager::NUMBER_OF_L0_DATA_SOURCES; i++) {
		L0Subevents[i]->destroy();
	}

	for (int ID = 0;
			ID < SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
			ID++) {
		cream::LkrFragment* fragment = zSuppressedLkrFragmentsByLocalCREAMID[ID];
		if (fragment != nullptr) {
			delete fragment;
			zSuppressedLkrFragmentsByLocalCREAMID[ID] = nullptr;
		}
	}

	for (auto& pair : nonSuppressedLkrFragmentsByCrateCREAMID) {
		delete pair.second;
	}
	nonSuppressedLkrFragmentsByCrateCREAMID.clear();

	reset();
}

std::string Event::getMissingSourceIDs() {
	/*
	 * Find the missing sourceIDs
	 */
	std::stringstream missingIDs;
	for (int i = SourceIDManager::NUMBER_OF_L0_DATA_SOURCES - 1; i >= 0; i--) {
		if (SourceIDManager::getExpectedPacksBySourceNum(i)
				!= getL0SubeventBySourceIDNum(i)->getNumberOfFragments()) {
			missingIDs << (int) SourceIDManager::SourceNumToID(i) << "; ";
		}
	}
	for (int i = SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT
			- 1; i >= 0; i--) {
		if (zSuppressedLkrFragmentsByLocalCREAMID[i] == nullptr) {
			std::pair<uint8_t, uint8_t> crateAndCream =
					SourceIDManager::getCrateAndCREAMIDByLocalID(i);

			missingIDs << "crate " << (int) crateAndCream.first << "/cream "
					<< (int) crateAndCream.second << "; ";
		}
	}

	std::stringstream dump;
	dump << "Burst:\t" << getBurstID() << "\tEvent:\t" << getEventNumber()
			<< "\tTS:\t" << getTimestamp() << "\tMissing:\t";
	dump << missingIDs.str();
	DataDumper::printToFile("unfinishedEvents", "/tmp/farm-logs", dump.str());

	return missingIDs.str();
}

} /* namespace na62 */
