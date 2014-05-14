/*
 * Event.cpp
 *
 *  Created on: Jan 18, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "Event.h"

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <utility>
#include <fstream>
#include <boost/thread/pthread/mutex.hpp>

#include "../l0/MEPEvent.h"
#include "../l0/MEP.h"
#include "../l0/Subevent.h"
#include "../LKr/LKRMEP.h"
#include "../utils/DataDumper.h"

namespace na62 {
boost::mutex unfinishedEventsIOMutex;

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
#ifdef MEASURE_TIME
	if (firstEventPartAddedTime_.is_stopped()) {
		firstEventPartAddedTime_.start();
	}
#endif
//	if (eventNumber_ != l0Event->getEventNumber()) {
//		LOG(ERROR)<<
//		"Trying to add MEPEvent with eventNumber " + boost::lexical_cast<std::string>(l0Event->getEventNumber())
//		+ " to an Event with eventNumber " + boost::lexical_cast<std::string>(eventNumber_) + ". Will ignore the MEPEvent!";
//		delete l0Event;
//		return false;
//	}

	if (numberOfL0Events_ == 0) {
		lastEventOfBurst_ = l0Event->isLastEventOfBurst();
		setBurstID(burstID);
	} else {
		if (l0Event->isLastEventOfBurst() != lastEventOfBurst_) {
			destroy();
			LOG(ERROR)<<"MEPE Events  'lastEvenOfBurst' flag discords with the flag of the Event with the same eventNumber.";
			return addL0Event(l0Event, burstID);
		}

		if (burstID != getBurstID()) {
			/*
			 * Append the unfinished event information to a file for debugging
			 */
			boost::lock_guard<boost::mutex> lock(unfinishedEventsIOMutex);
			std::ofstream myfile;
			myfile.open("unfinishedEventNumbers",
					std::ios::out | std::ios::app);

			if (!myfile.good()) {
				std::cerr << "Unable to write to file "
						<< "unfinishedEventNumbers" << std::endl;
			} else {
				myfile << burstID << "\t" << getEventNumber() << "\t"
						<< getTimestamp();

				for (int localCreamID =
						SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT
								- 1; localCreamID != -1; localCreamID--) {
					if (getZSuppressedLKrEvent(localCreamID) == nullptr) {
						auto crateAndCreamID =
								SourceIDManager::getCrateAndCREAMIDByLocalID(
										localCreamID);
						myfile << "\t" << (int)crateAndCreamID.first << "\t"
								<< (int)crateAndCreamID.second;
					}
				}

				myfile << std::endl;
			}
			myfile.close();

			/*
			 * Event not build during last burst -> destroy it!
			 */
			LOG(ERROR)<<
			"Overwriting unfinished event from Burst " + boost::lexical_cast<std::string>((int ) getBurstID()) + "! Eventnumber: "
			+ boost::lexical_cast<std::string>((int ) getEventNumber());
			destroy();
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

	if (subevent->getNumberOfParts()
			>= SourceIDManager::getExpectedPacksByEventID(
					l0Event->getSourceID())) {
		/*
		 * Already received enough packets from that sourceID! It seems like this is an old event from the last burst -> destroy it!
		 */
		LOG(ERROR)<<"Event number " << l0Event->getEventNumber() << " already received from source " << ((int) l0Event->getSourceID());
		destroy();
		return addL0Event(l0Event, burstID);
	}

	subevent->addEventPart(l0Event);
	numberOfL0Events_++;

#ifdef MEASURE_TIME
	if (numberOfL0Events_
			== SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT) {
		l0BuildingTime_ = firstEventPartAddedTime_.elapsed().wall / 1E3;

		return true;
	}
	return false;
#else
	return numberOfL0Events_
			== SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT;
#endif
}

bool Event::addLKREvent(cream::LKREvent* lkrEvent) {
	if (!L1Processed_) {
		std::string fileName = "CREAM_NotRequested_EN-"
				+ std::to_string(lkrEvent->getEventNumber()) + "_CRATE-"
				+ std::to_string(lkrEvent->getCrateID())+"_SLOT-"
				+ std::to_string(lkrEvent->getCREAMID());
		DataDumper::dumpToFile(fileName, "errorEventDump/",
				lkrEvent->getMep()->getRawData(),
				lkrEvent->getMep()->getRawLength());

		LOG(ERROR)<<
		"Received LKR data with EventNumber " + boost::lexical_cast<std::string>((int ) lkrEvent->getEventNumber()) + ", crateID "
		+ boost::lexical_cast<std::string>((int ) lkrEvent->getCrateID()) + " and CREAMID "
		+ boost::lexical_cast<std::string>((int ) lkrEvent->getCREAMID())
		+ " before requesting it. Will ignore it as it seems to come from last burst.";
		delete lkrEvent;
		return false;
	}

	if (eventNumber_ != lkrEvent->getEventNumber()) {
		std::string fileName = "CREAM_Twice_EN-"
				+ std::to_string(lkrEvent->getEventNumber()) + "_CRATE-"
				+ std::to_string(lkrEvent->getCrateID())+"_SLOT-"
				+ std::to_string(lkrEvent->getCREAMID());
		DataDumper::dumpToFile(fileName, "errorEventDump/",
				lkrEvent->getMep()->getRawData(),
				lkrEvent->getMep()->getRawLength());

//		for (int sourceIDNum = SourceIDManager::NUMBER_OF_L0_DATA_SOURCES - 1;
//				sourceIDNum >= 0; sourceIDNum--) {
//			l0::Subevent* subevent = getL0SubeventBySourceIDNum(sourceIDNum);
//
//			for (int partNum = subevent->getNumberOfParts() - 1; partNum >= 0;
//					partNum--) {
//				l0::MEPEvent* e = subevent->getPart(partNum);
//				fileName += "_L0_SOURCE-" + std::to_string(e->getSourceID())
//						+ "_" + std::to_string(partNum);
//				DataDumper::dumpToFile(fileName, "errorEventDump/",
//						e->getMep()->getRawData(), e->getMep()->getRawLength());
//			}
//		}

		LOG(ERROR)<<
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
		std::map<uint16_t, cream::LKREvent*>::iterator lb =
				nonSuppressedLKrEventsByCrateCREAMID.lower_bound(crateCREAMID);

		if (lb != nonSuppressedLKrEventsByCrateCREAMID.end()
				&& !(nonSuppressedLKrEventsByCrateCREAMID.key_comp()(
						crateCREAMID, lb->first))) {
			LOG(ERROR)<<
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
		return nonSuppressedLKrEventsByCrateCREAMID.size()
				== nonZSuppressedDataRequestedNum;
	} else {
		uint16_t localCreamID = SourceIDManager::getLocalCREAMID(
				lkrEvent->getCrateID(), lkrEvent->getCREAMID());
		/*
		 * This must be a zero suppressed event
		 */
		cream::LKREvent* oldEvent =
				zSuppressedLKrEventsByCrateCREAMID[localCreamID];

		if (oldEvent != NULL) {
			LOG(ERROR)<<
			"LKr event with EventNumber " + boost::lexical_cast<std::string>((int ) lkrEvent->getEventNumber()) + ", crateID "
			+ boost::lexical_cast<std::string>((int ) lkrEvent->getCrateID()) + " and CREAMID "
			+ boost::lexical_cast<std::string>((int ) lkrEvent->getCREAMID()) + " received twice! Will delete the whole event!";
			destroy();
			delete lkrEvent;
			return false;
		}

		zSuppressedLKrEventsByCrateCREAMID[localCreamID] = lkrEvent;
		numberOfCREAMEvents_++;

#ifdef MEASURE_TIME
		if (numberOfCREAMEvents_ == SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT) {
			l1BuildingTime_ = firstEventPartAddedTime_.elapsed().wall/ 1E3-l1ProcessingTime_;
			return true;
		}
		return false;
#else
		return numberOfCREAMEvents_
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
