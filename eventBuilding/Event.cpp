/*
 * Event.cpp
 *
 *  Created on: Jan 18, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "Event.h"

#include <cstdbool>
#include <sstream>

#include "../exceptions/NA62Error.h"
#include "EventPool.h"

#include <sys/types.h>
#include <iostream>
#include <string>
#include <utility>

#include "../structs/L0TPHeader.h"
#include "../utils/DataDumper.h"
#include "../l0/MEPFragment.h"
#include "../l0/Subevent.h"
#include "../options/Logging.h"
#include "UnfinishedEventsCollector.h"
namespace na62 {
bool Event::printMissingSourceIDs_ = true;
bool Event::writeBrokenCreamInfo_ = false;

std::atomic<uint64_t>* Event::MissingEventsBySourceNum_;
//std::atomic<uint64_t>** Event::ReceivedEventsBySourceNumBySubId_;
std::atomic<uint64_t> Event::nonRequestsCreamFramesReceived_;

Event::Event(uint_fast32_t eventNumber) :
		eventNumber_(eventNumber), numberOfL0Fragments_(0), numberOfCREAMFragments_(
				0), burstID_(0), triggerTypeWord_(0), timestamp_(0), finetime_(
				0), SOBtimestamp_(0), processingID_(0), requestZeroSuppressedCreamData_(
		false), nonZSuppressedDataRequestedNum(0), L1Processed_(false), L2Accepted_(
		false), unfinished_(false), lastEventOfBurst_(
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
//	throw NA62Error(
//			"An Event-Object should not be deleted! Use EventPool::FreeEvent instead so that it can be reused by the EventBuilder!");

	for (uint_fast8_t i = 0; i < SourceIDManager::NUMBER_OF_L0_DATA_SOURCES;
			i++) {
//		L0Subevents[i]->destroy();
		delete L0Subevents[i];
	}
	delete[] L0Subevents;

	for (int ID = 0;
			ID < SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
			ID++) {
		cream::LkrFragment* event = zSuppressedLkrFragmentsByLocalCREAMID[ID];
		if (event != NULL) {
			delete event;
		}
	}
	delete[] zSuppressedLkrFragmentsByLocalCREAMID;
}

void Event::initialize(bool printMissingSourceIDs, bool writeBrokenCreamInfo) {
	MissingEventsBySourceNum_ =
			new std::atomic<uint64_t>[SourceIDManager::NUMBER_OF_L0_DATA_SOURCES
					+ 1];
//	ReceivedEventsBySourceNumBySubId_ =
//			new std::atomic<uint64_t>*[SourceIDManager::NUMBER_OF_L0_DATA_SOURCES
//					+ 1];

	for (int i = 0; i != SourceIDManager::NUMBER_OF_L0_DATA_SOURCES; i++) {
		MissingEventsBySourceNum_[i] = 0;

//		ReceivedEventsBySourceNumBySubId_[i] =
//				new std::atomic<uint64_t>[SourceIDManager::getExpectedPacksBySourceNum(
//						i)];
//		for (int f = 0; f != SourceIDManager::getExpectedPacksBySourceNum(i);
//				f++) {
//			ReceivedEventsBySourceNumBySubId_[i][f] = 0;
//		}
	}

	printMissingSourceIDs_ = printMissingSourceIDs;

	writeBrokenCreamInfo_ = writeBrokenCreamInfo;
}

/**
 * Process data coming from the TEL boards
 */
bool Event::addL0Event(l0::MEPFragment* fragment, uint_fast32_t burstID) {
#ifdef MEASURE_TIME
	if (firstEventPartAddedTime_.is_stopped()) {
		firstEventPartAddedTime_.start();
	}
#endif

	unfinished_ = true;
	if (numberOfL0Fragments_ == 0) {
		lastEventOfBurst_ = fragment->isLastEventOfBurst();
		setBurstID(burstID);
	} else {
		if (burstID != getBurstID()) {
			if (unfinishedEventMutex_.try_lock()) {
				/*
				 * Event not build during last burst -> destroy it!
				 */
				if (printMissingSourceIDs_) {
					LOG_INFO<< "Overwriting unfinished event from Burst "
					<< (int) getBurstID() << " Eventnumber "
					<< (int) getEventNumber()
					<< ENDL;
				}
				EventPool::freeEvent(this);
				unfinishedEventMutex_.unlock();
			} else {
				/*
				 * If we didn't get the lock: wait for the other thread to free the event
				 */
				tbb::spin_mutex::scoped_lock my_lock(unfinishedEventMutex_);
			}
			/*
			 * Add the event after this or another thread has destroyed this event
			 */
			return addL0Event(fragment, burstID);
		}
	}

	l0::Subevent* subevent = L0Subevents[fragment->getSourceIDNum()];

	if (!subevent->addFragment(fragment)) {
		/*
		 * Already received enough packets from that sourceID! It seems like this is an old event from the last burst -> destroy it!
		 */
		if (printMissingSourceIDs_) {
			LOG_ERROR<< "Already received all fragments from sourceID "
			<< ((int) fragment->getSourceID()) << " sourceSubID " << ((int) fragment->getSourceSubID())
			<< ENDL;
		}

		delete fragment;
		return false;
	}

	int currentValue = numberOfL0Fragments_.fetch_add(1,
			std::memory_order_release) + 1;

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

	const uint_fast16_t crateCREAMID = fragment->getCrateCREAMID();
	/*
	 * We were waiting for non zero suppressed data
	 */
	auto lb = nonSuppressedLkrFragmentsByCrateCREAMID.lower_bound(crateCREAMID);

	if (lb != nonSuppressedLkrFragmentsByCrateCREAMID.end()
			&& !(nonSuppressedLkrFragmentsByCrateCREAMID.key_comp()(
					crateCREAMID, lb->first))) {
		if (unfinishedEventMutex_.try_lock()) {
			if (printMissingSourceIDs_) {
				LOG_INFO<< "Non zero suppressed LKr event with EventNumber "
				<< (int) fragment->getEventNumber() << ", crateID "
				<< (int) fragment->getCrateID() << " and CREAMID "
				<< (int) fragment->getCREAMID()
				<< " received twice! Will delete the whole event!" << ENDL;
			}
			nonRequestsCreamFramesReceived_.fetch_add(1, std::memory_order_relaxed);

			EventPool::freeEvent(this);
			unfinishedEventMutex_.unlock();
		}
		delete fragment;
		return false;
	} else {
		/*
		 * Event does not yet exist -> add it to the map
		 */
		nonSuppressedLkrFragmentsByCrateCREAMID.insert(lb,
				std::map<uint_fast16_t, cream::LkrFragment*>::value_type(crateCREAMID,
						fragment));
	}
// TODO: this must be synchronized
	return nonSuppressedLkrFragmentsByCrateCREAMID.size()
			== nonZSuppressedDataRequestedNum;
}

/**
 * Process data coming from the CREAMs
 */
bool Event::addLkrFragment(cream::LkrFragment* fragment, uint sourceIP) {
	if (!L1Processed_) {
		if (printMissingSourceIDs_) {
			LOG_ERROR<< "Received LKR data with EventNumber "
			<< (int) fragment->getEventNumber() << ", crateID "
			<< (int) fragment->getCrateID() << " and CREAMID "
			<< (int) fragment->getCREAMID()
			<< " before requesting it. Will ignore it as it seems to come from last burst ( current burst is "
			<< getBurstID() << ")"
			<< ENDL;
		}
		nonRequestsCreamFramesReceived_.fetch_add(1, std::memory_order_relaxed);

		if(writeBrokenCreamInfo_) {
			std::stringstream dump;
			dump << getBurstID() << "\t" << getEventNumber()
			<< "\t" << fragment->getTimestamp() << "\t" << fragment->getEventLength()<<"\t"<<std::hex << ntohl(sourceIP);
			DataDumper::printToFile("nonRequestedCreamData", "/tmp/farm-logs", dump.str());
		}

		delete fragment;
		return false;
	}

	if (eventNumber_ != fragment->getEventNumber()) {
		LOG_ERROR<< "Trying to add LkrFragment with eventNumber "
		+ std::to_string(
				fragment->getEventNumber())
		+ " to an Event with eventNumber "
		+ std::to_string(eventNumber_)
		+ ". Will ignore the LkrFragment!"
		<< ENDL;
		delete fragment;
		return false;
	}

	if (nonZSuppressedDataRequestedNum != 0) {
		return storeNonZSuppressedLkrFragemnt(fragment);
	} else {
		/*
		 * ZSuppressed data received
		 */
		uint_fast16_t localCreamID = SourceIDManager::getLocalCREAMID(
				fragment->getCrateID(), fragment->getCREAMID());
		/*
		 * This must be a zero suppressed event
		 */
		cream::LkrFragment* oldEvent =
		zSuppressedLkrFragmentsByLocalCREAMID[localCreamID];

		if (oldEvent != NULL) {
			if (unfinishedEventMutex_.try_lock()) {
				if (printMissingSourceIDs_) {
					LOG_ERROR<< "LKr event with EventNumber "
					+ std::to_string(
							(int) fragment->getEventNumber())
					+ ", crateID "
					+ std::to_string(
							(int) fragment->getCrateID())
					+ " and CREAMID "
					+ std::to_string(
							(int) fragment->getCREAMID())
					+ " received twice! Will delete the whole event!"<<ENDL;

				}

				nonRequestsCreamFramesReceived_.fetch_add(1, std::memory_order_relaxed);

				if(writeBrokenCreamInfo_) {
					std::stringstream dump;
					dump << getBurstID() << "\t" << getEventNumber()
					<< "\t" << getTimestamp() << "\t" << sourceIP;
					DataDumper::printToFile("creamDataReceivedTwice", "/tmp/farm-logs", dump.str());
				}

				EventPool::freeEvent(this);
				unfinishedEventMutex_.unlock();
			}
			delete fragment;
			return false;
		}
		zSuppressedLkrFragmentsByLocalCREAMID[localCreamID] = fragment;

		int numberOfStoredCreamFragments = numberOfCREAMFragments_.fetch_add(1,
				std::memory_order_release) + 1;

#ifdef MEASURE_TIME
		if (numberOfStoredCreamFragments == SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT) {
			l1BuildingTime_ = firstEventPartAddedTime_.elapsed().wall/ 1E3-(l1ProcessingTime_+l0BuildingTime_);
			return true;
		}
		return false;
#else

		return numberOfStoredCreamFragments
		== SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
#endif
	}
}

void Event::reset() {
	numberOfL0Fragments_ = 0;
	numberOfCREAMFragments_ = 0;
	burstID_ = 0;
	triggerTypeWord_ = 0;
	timestamp_ = 0;
	finetime_ = 0;
	processingID_ = 0;
	requestZeroSuppressedCreamData_ = false;
	L1Processed_ = false;
	L2Accepted_ = false;
	unfinished_ = false;
	lastEventOfBurst_ = false;
	nonZSuppressedDataRequestedNum = 0;
}

void Event::destroy() {
	tbb::spin_mutex::scoped_lock my_lock(destroyMutex_);
#ifdef MEASURE_TIME
	firstEventPartAddedTime_.stop();
#endif

	for (uint_fast8_t i = 0; i != SourceIDManager::NUMBER_OF_L0_DATA_SOURCES;
			i++) {
		L0Subevents[i]->destroy();
	}

	for (int ID = 0;
			ID != SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
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

//std::map<uint, std::map<uint, uint>> Event::getReceivedSourceIDsSourceSubIds() {
//	std::map<uint, std::map<uint, uint>> receivedSourceIdsSubIds;
//
//	for (int sourceNum = SourceIDManager::NUMBER_OF_L0_DATA_SOURCES - 1;
//			sourceNum >= 0; sourceNum--) {
//
//		l0::Subevent* subevent = getL0SubeventBySourceIDNum(sourceNum);
////		LOG_INFO<< "+++++++++RECEIVED SOURCEIDs   " << " " << std::hex << (int) SourceIDManager::sourceNumToID(sourceNum)
////		<< std::dec << " " << (int) SourceIDManager::getExpectedPacksBySourceNum(sourceNum) << " "
////		<< (int) subevent->getNumberOfFragments() << ENDL;
//
//		for (int f = 0; f != subevent->getNumberOfFragments(); f++) {
//			receivedSourceIdsSubIds[SourceIDManager::sourceNumToID(sourceNum)][subevent->getFragment(
//					f)->getSourceSubID()] = 1;
//			ReceivedEventsBySourceNumBySubId_[sourceNum][subevent->getFragment(
//					f)->getSourceSubID()].fetch_add(1,
//					std::memory_order_relaxed);
//
////			LOG_INFO<< "SETTO (" << std::hex << (int)SourceIDManager::sourceNumToID(sourceNum)<< std::dec
////			<< "," << (uint)subevent->getFragment(f)->getSourceSubID()
////			<< ") a " << receivedSourceIdsSubIds[SourceIDManager::sourceNumToID(sourceNum)][subevent->getFragment(f)->getSourceSubID()] <<ENDL;
////			LOG_INFO<< "INCREMENTO (" << std::hex << (int)SourceIDManager::sourceNumToID(sourceNum)<< std::dec
////			<< "," << (uint)subevent->getFragment(f)->getSourceSubID() << ") a "
////			<< ReceivedEventsBySourceNumBySubId_[sourceNum][subevent->getFragment(f)->getSourceSubID()] <<ENDL;
//		}
//	}
//	return receivedSourceIdsSubIds;
//}

std::map<uint, std::vector<uint>> Event::getMissingSourceIDs() {
	std::map<uint, std::vector<uint>> missingIds;
	if (!L1Processed_) {
		for (int sourceNum = SourceIDManager::NUMBER_OF_L0_DATA_SOURCES - 1;
				sourceNum >= 0; sourceNum--) {
			l0::Subevent* subevent = getL0SubeventBySourceIDNum(sourceNum);
//			LOG_INFO<< "+++++++++ MISSING SOURCEIDs   " << " " << std::hex
//			<< (int) SourceIDManager::sourceNumToID(sourceNum) << std::dec
//			<< " " << (int) SourceIDManager::getExpectedPacksBySourceNum(sourceNum)
//			<< " " << (int) subevent->getNumberOfFragments() << ENDL;

			if (SourceIDManager::getExpectedPacksBySourceNum(sourceNum)
					!= subevent->getNumberOfFragments()) {
				MissingEventsBySourceNum_[sourceNum].fetch_add(1,
						std::memory_order_relaxed);
				missingIds[SourceIDManager::sourceNumToID(sourceNum)] =
						subevent->getMissingSourceSubIds();

				for (int f = 0; f != subevent->getNumberOfFragments(); f++) {
					UnfinishedEventsCollector::addReceivedSubSourceIdFromUnfinishedEvent(
							sourceNum,
							subevent->getFragment(f)->getSourceSubID());
				}
			}
		}
	}
	return missingIds;
}

std::map<uint, std::vector<uint>> Event::getMissingCreams() {
	std::map<uint, std::vector<uint>> missingCratsAndCreams;
	if (L1Processed_
			&& numberOfCREAMFragments_
					!= SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT) {
		MissingEventsBySourceNum_[SourceIDManager::NUMBER_OF_L0_DATA_SOURCES].fetch_add(
				1, std::memory_order_relaxed);

		uint crateID = 0xFFFFFFFF;
		for (int i = 0;
				i != SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
				i++) {
			if (zSuppressedLkrFragmentsByLocalCREAMID[i] == nullptr) {
				std::pair<uint_fast8_t, uint_fast8_t> crateAndCream =
						SourceIDManager::getCrateAndCREAMIDByLocalID(i);
				missingCratsAndCreams[crateAndCream.first].push_back(
						crateAndCream.second);
			}
		}
	}
	return missingCratsAndCreams;
}

uint_fast8_t Event::readTriggerTypeWordAndFineTime() {
	/*
	 * Read the L0 trigger type word and the fine time from the L0TP data
	 */
	if (SourceIDManager::L0TP_ACTIVE) {
		l0::MEPFragment* L0TPEvent = getL0TPSubevent()->getFragment(0);
		L0TpHeader* L0TPData = (L0TpHeader*) L0TPEvent->getPayload();
		setFinetime(L0TPData->refFineTime);
		setl0TriggerTypeWord(L0TPData->l0TriggerType);
		return L0TPData->l0TriggerType;
	}
	return 1;
}

} /* namespace na62 */
