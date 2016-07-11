/*
 * Event.cpp
 *
 *  Created on: Jan 18, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "Event.h"

#include <netinet/in.h>
#include <sys/types.h>
#include <cstdbool>
#include <functional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../monitoring/DetectorStatistics.h"
#include "../exceptions/CommonExceptions.h"
#include "../l0/MEP.h"
#include "../l0/MEPFragment.h"
#include "../l0/Subevent.h"
#include "../l1/MEP.h"
#include "../l1/MEPFragment.h"
#include "../l1/Subevent.h"

#include "../structs/DataContainer.h"
#include "../structs/L0TPHeader.h"
#include "../utils/DataDumper.h"
#include "EventPool.h"
#include "UnfinishedEventsCollector.h"

namespace na62 {

//std::atomic<uint64_t>** Event::ReceivedEventsBySourceNumBySubId_;
std::atomic<uint64_t>* Event::MissingEventsBySourceNum_;
std::atomic<uint64_t>* Event::MissingL1EventsBySourceNum_;
std::atomic<uint64_t> Event::nonRequestsL1FramesReceived_;
bool Event::printCompletedSourceIDs_ = false;

Event::Event(uint_fast32_t eventNumber) :
		eventNumber_(eventNumber), numberOfL0Fragments_(0), numberOfMEPFragments_(
				0), burstID_(0), triggerTypeWord_(0), triggerFlags_(0), triggerDataType_(0), timestamp_(
				0), finetime_(0), SOBtimestamp_(0), processingID_(0), requestZeroSuppressedCreamData_(
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
				SourceIDManager::getExpectedPacksBySourceNum(i),
				SourceIDManager::sourceNumToID(i));
	}

	L1Subevents = new l1::Subevent*[SourceIDManager::NUMBER_OF_L1_DATA_SOURCES];
	for (int i = SourceIDManager::NUMBER_OF_L1_DATA_SOURCES - 1; i >= 0; i--) {
		/*
		 * Initialize subevents[sourceID] with new Subevent(Number of expected Events)
		 */
		L1Subevents[i] = new l1::Subevent(
				SourceIDManager::getExpectedL1PacksBySourceNum(i),
				SourceIDManager::l1SourceNumToID(i));
	}
}

Event::Event(EVENT_HDR* serializedEvent, bool onlyL0) :
	eventNumber_(serializedEvent->eventNum), numberOfL0Fragments_(0), numberOfMEPFragments_(0), burstID_(serializedEvent->burstID),
			triggerTypeWord_(serializedEvent->triggerWord), triggerFlags_(0),
			timestamp_(serializedEvent->timestamp), finetime_(serializedEvent->fineTime), SOBtimestamp_(serializedEvent->SOBtimestamp), processingID_(serializedEvent->processingID),
			requestZeroSuppressedCreamData_(false), nonZSuppressedDataRequestedNum(0), L1Processed_(false), L2Accepted_(false), unfinished_(false), lastEventOfBurst_(false) {


	std::cout << "Creating event with ID " << (int) eventNumber_ << std::endl;


	// Helper variables to navigate through serialized event;
	uint sizeOfPointerTable  = 4 * (SourceIDManager::NUMBER_OF_L0_DATA_SOURCES + SourceIDManager::NUMBER_OF_L1_DATA_SOURCES) ;
	uint pointerTableOffset = sizeof(EVENT_HDR);
	uint eventOffset = sizeof(EVENT_HDR) + sizeOfPointerTable;

	char* serializedBuf = reinterpret_cast<char*>(serializedEvent);
	if(!onlyL0) {
		L1Processed_ = true;
	}
	/*
	 * Initialize subevents at the existing sourceIDs as position for L0 detectors
	 */
	L0Subevents = new l0::Subevent*[SourceIDManager::NUMBER_OF_L0_DATA_SOURCES];
	for (int i = SourceIDManager::NUMBER_OF_L0_DATA_SOURCES - 1; i >= 0; i--) {
		/*
		 * Initialize subevents[sourceID] with new Subevent(Number of expected Events)
		 */

		std::cout << "Build subevent for det 0x" << std::hex << (uint) SourceIDManager::sourceNumToID(i) << std::dec << std::endl;


		L0Subevents[i] = new l0::Subevent(
				SourceIDManager::getExpectedPacksBySourceNum(i),
				SourceIDManager::sourceNumToID(i));
	}
	std::cout << "Now populate the fragments" << std::endl;

	EVENT_DATA_PTR* sourceIdAndOffsets = serializedEvent->getDataPointer();
	for(int sourceNum=0; sourceNum!=SourceIDManager::NUMBER_OF_L0_DATA_SOURCES; sourceNum++){
		EVENT_DATA_PTR sourceIdAndOffset = sourceIdAndOffsets[sourceNum];
		std::cout << "Found detector " << std::hex << (int) sourceIdAndOffset.sourceID << " Starting at: " << std::dec << sourceIdAndOffset.offset << " in the serialized event." << std::dec << std::endl;
		const char* detectorData = serializedBuf + (sourceIdAndOffset.offset * 4);
		//const char* detectorData = serializedBuf + (sourceIdAndOffset.offset);
		l0::Subevent * se = L0Subevents[SourceIDManager::sourceIDToNum(sourceIdAndOffset.sourceID)];
		int fragOffset = 0;
		for (int j = 0 ; j < se->getNumberOfExpectedFragments(); ++j) {
			std::cout << "Recreating fragment: " << std::dec << j << std::endl;
			const L0_BLOCK_HDR* l0b = reinterpret_cast<const L0_BLOCK_HDR*>(detectorData+fragOffset);
			const l0::MEPFragment_HDR* fragData = reinterpret_cast<const l0::MEPFragment_HDR*> (detectorData+fragOffset) ;
			std::cout << "Create MEPFragment " << j << " and size " << (int) l0b->dataBlockSize << " with subID 0x" << std::hex << (int) l0b->sourceSubID << std::dec << std::endl;
			l0::MEPFragment * myFrag = new l0::MEPFragment(fragData, eventNumber_, sourceIdAndOffset.sourceID, l0b->sourceSubID);
			se->addFragment(myFrag);
			std::cout << "Added fragment to subevent 0x" << std::hex << (int) se->getSourceID() << std::dec << std::endl;
			fragOffset += l0b->dataBlockSize;
		}
	}

	if (!onlyL0) {
		/*
		 * Initialize subevents at the existing sourceIDs as position for L1 detectors
		 */

		L1Subevents = new l1::Subevent*[SourceIDManager::NUMBER_OF_L1_DATA_SOURCES];
		for (int i = SourceIDManager::NUMBER_OF_L1_DATA_SOURCES - 1; i >= 0; i--) {
			/*
			 * Initialize subevents[sourceID] with new Subevent(Number of expected Events)
			 */
			L1Subevents[i] = new l1::Subevent(
					SourceIDManager::getExpectedL1PacksBySourceNum(i),
					SourceIDManager::l1SourceNumToID(i));
		}

		for(int sourceNum=SourceIDManager::NUMBER_OF_L0_DATA_SOURCES; sourceNum!=SourceIDManager::NUMBER_OF_L0_DATA_SOURCES + SourceIDManager::NUMBER_OF_L1_DATA_SOURCES; sourceNum++){
			EVENT_DATA_PTR sourceIdAndOffset = sourceIdAndOffsets[sourceNum];
			const char* detectorData = serializedBuf + (sourceIdAndOffset.offset * 4);
			l1::Subevent * se = L1Subevents[SourceIDManager::l1SourceIDToNum(sourceIdAndOffset.sourceID)];
			int fragOffset = 0;
			for (int j = 0 ; j < se->getNumberOfExpectedFragments(); ++j) {
				const l1::L1_EVENT_RAW_HDR * fragData = reinterpret_cast<const l1::L1_EVENT_RAW_HDR*>(detectorData+fragOffset) ;
				l1::MEPFragment * myFrag = new l1::MEPFragment(NULL, fragData);
				se->addFragment(myFrag);
				fragOffset += fragData->numberOf4BWords * 4;
			}
		}
	}

}

Event::~Event() {
	LOG_INFO("Destructor of Event "<< (int) this->getEventNumber());

}

void Event::initialize(bool printCompletedSourceIDs) {

	Event::printCompletedSourceIDs_ = printCompletedSourceIDs;
	Event::MissingEventsBySourceNum_ = new std::atomic<uint64_t>[SourceIDManager::NUMBER_OF_L0_DATA_SOURCES];
	Event::MissingL1EventsBySourceNum_ = new std::atomic<uint64_t>[SourceIDManager::NUMBER_OF_L1_DATA_SOURCES];

	for (size_t i = 0; i != SourceIDManager::NUMBER_OF_L0_DATA_SOURCES; ++i)
		MissingEventsBySourceNum_[i] = 0;
	for (size_t i = 0; i != SourceIDManager::NUMBER_OF_L1_DATA_SOURCES; ++i)
		MissingL1EventsBySourceNum_[i] = 0;
}

/**
 * Process data coming from the TEL boards
 */
bool Event::addL0Fragment(l0::MEPFragment* fragment, uint_fast32_t burstID) {
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
		if (!lastEventOfBurst_)
			lastEventOfBurst_ = fragment->isLastEventOfBurst(); // work around STRAWs bug
		if (burstID > getBurstID()) {
			if (unfinishedEventMutex_.try_lock()) {
				LOG_ERROR("Identified non cleared event " << (uint) getEventNumber() << " from previous burst!");
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
			return addL0Fragment(fragment, burstID);
		} else if (burstID < getBurstID()) {
			LOG_ERROR("Received fragment from a previous burst for event " << (uint) getEventNumber());
			delete fragment;
			return false;
		}
	}

	l0::Subevent* subevent = L0Subevents[fragment->getSourceIDNum()];

	if (!subevent->addFragment(fragment)) {
		/*
		 * Already received enough packets from that sourceID! Eliminate fragment
		 */
#ifdef USE_ERS
		ers::error(DuplicateFragment(ERS_HERE, SourceIDManager::sourceIdToDetectorName(fragment->getSourceID()), fragment->getSourceSubID(), this->getEventNumber()));
#else
		LOG_ERROR("type = BadEv : Already received all fragments from sourceID 0x" << std::hex << ((int) fragment->getSourceID()) << " sourceSubID 0x" << ((int) fragment->getSourceSubID()) << " for event " << std::dec << (int)(this->getEventNumber()));
#endif
		delete fragment;
		return false;
	}

	uint currentValue = numberOfL0Fragments_.fetch_add(1,
			std::memory_order_release) + 1;

#ifdef MEASURE_TIME
	bool result = currentValue == SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT;
	if (currentValue
			== SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT) {
		l0BuildingTime_ = firstEventPartAddedTime_.elapsed().wall / 1E3;
		if (currentValue
				> SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT)
			LOG_ERROR("Too many L0 Packets:" << currentValue << "/" << SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT);
	}
	return result;

#else
	return currentValue
	== SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT;
#endif
}

bool Event::storeNonZSuppressedLkrFragemnt(l1::MEPFragment* fragment) {
	/*
	 * The received LkrFragment should be nonZSuppressed data
	 */

	const uint_fast16_t crateCREAMID = fragment->getSourceSubID();

	/*
	 * We were waiting for non zero suppressed data
	 */
	auto lb = nonSuppressedLkrFragmentsByCrateCREAMID.lower_bound(crateCREAMID);

	if (lb != nonSuppressedLkrFragmentsByCrateCREAMID.end()
			&& !(nonSuppressedLkrFragmentsByCrateCREAMID.key_comp()(
					crateCREAMID, lb->first))) {
		if (unfinishedEventMutex_.try_lock()) {
			LOG_INFO("Non zero suppressed LKr event with EventNumber " << (int) fragment->getEventNumber() << ", crate/creamID " << std::hex << (int) fragment->getSourceSubID() << std::dec << " received twice! Will delete the whole event!");
			nonRequestsL1FramesReceived_.fetch_add(1,
					std::memory_order_relaxed);

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
				std::map<uint_fast16_t, l1::MEPFragment*>::value_type(
						crateCREAMID, fragment));
	}
// TODO: this must be synchronized
	return nonSuppressedLkrFragmentsByCrateCREAMID.size()
			== nonZSuppressedDataRequestedNum;
}

/**
 * Process data coming from the CREAMs
 */
bool Event::addL1Fragment(l1::MEPFragment* fragment) {
	if (!L1Processed_) {
#ifdef USE_ERS
		ers::error(UnrequestedFragment(ERS_HERE, this->getEventNumber(), SourceIDManager::sourceIdToDetectorName(fragment->getSourceID()), fragment->getSourceSubID()));
#else
		LOG_ERROR("type = BadEv : Received L1 data from " << std::hex << (int) fragment->getSourceID() << ":"<< (int) fragment->getSourceSubID() << " with EventNumber " << std::dec << (int) fragment->getEventNumber() << " before requesting it. Will ignore it as it may come from last burst");
#endif
		nonRequestsL1FramesReceived_.fetch_add(1, std::memory_order_relaxed);

		delete fragment;
		return false;
	}

	if (nonZSuppressedDataRequestedNum != 0) {
		return storeNonZSuppressedLkrFragemnt(fragment);
	} else {
		l1::Subevent* subevent = L1Subevents[fragment->getSourceIDNum()];
		if (!subevent->addFragment(fragment)) {
			// don't know what to do with this fragment....
#ifdef USE_ERS
			ers::error(DuplicateFragment(ERS_HERE, SourceIDManager::sourceIdToDetectorName(fragment->getSourceID()), fragment->getSourceSubID(), this->getEventNumber()));
#else
			LOG_ERROR("type = BadEv : Already received all fragments from sourceID 0x"<< std::hex << ((int) fragment->getSourceID()) << " sourceSubID 0x" << ((int) fragment->getSourceSubID()) << " for event " << std::dec <<(int)(this->getEventNumber()));
#endif
			delete fragment;
			return false;
		}

		int numberOfMEPFragments = numberOfMEPFragments_.fetch_add(1,
				std::memory_order_release) + 1;

#ifdef MEASURE_TIME
		if (numberOfMEPFragments
				== SourceIDManager::NUMBER_OF_EXPECTED_L1_PACKETS_PER_EVENT) {
			l1BuildingTime_ = firstEventPartAddedTime_.elapsed().wall / 1E3
					- (l1ProcessingTime_ + l0BuildingTime_);
//			LOG_INFO("l1BuildingTime_ " << l1BuildingTime_);
			return true;
		}
		return false;
#else

		return numberOfMEPFragments
		== SourceIDManager::NUMBER_OF_EXPECTED_L1_PACKETS_PER_EVENT;
#endif
	}
}

void Event::reset() {
	numberOfL0Fragments_ = 0;
	numberOfMEPFragments_ = 0;
	burstID_ = 0;
	triggerTypeWord_ = 0;
	triggerFlags_ = 0;
	timestamp_ = 0;
	finetime_ = 0;
	triggerDataType_ = 0;
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
	//std::cout << "Event::destroy() for "<< (int) (this->getEventNumber())<< std::endl;
#ifdef MEASURE_TIME
	firstEventPartAddedTime_.stop();
#endif

	for (uint_fast8_t i = 0; i != SourceIDManager::NUMBER_OF_L0_DATA_SOURCES;
			i++) {
		L0Subevents[i]->destroy();
	}
	for (uint_fast8_t i = 0; i != SourceIDManager::NUMBER_OF_L1_DATA_SOURCES;
			i++) {
		L1Subevents[i]->destroy();
	}

	for (auto& pair : nonSuppressedLkrFragmentsByCrateCREAMID) {
		delete pair.second;
	}
	nonSuppressedLkrFragmentsByCrateCREAMID.clear();

	reset();
}

uint_fast8_t Event::readTriggerTypeWordAndFineTime() {
	/*
	 * Read the L0 trigger type word, trigger flags and the fine time from the L0TP data
	 */
	if (SourceIDManager::L0TP_ACTIVE) {
		l0::MEPFragment* L0TPEvent = getL0TPSubevent()->getFragment(0);
		L0TpHeader* L0TPData = (L0TpHeader*) L0TPEvent->getPayload();
		setFinetime(L0TPData->refFineTime);
		setTriggerDataType(L0TPData->dataType);
		setl0TriggerTypeWord(L0TPData->l0TriggerType);
		setTriggerFlags(L0TPData->l0TriggerFlags);
		return L0TPData->l0TriggerType;
		if(getL0TPSubevent()->getNumberOfFragments() > 0) {
			l0::MEPFragment* L0TPEvent = getL0TPSubevent()->getFragment(0);
			L0TpHeader* L0TPData = (L0TpHeader*) L0TPEvent->getPayload();
			setFinetime(L0TPData->refFineTime);
			setTriggerDataType(L0TPData->dataType);
			setl0TriggerTypeWord(L0TPData->l0TriggerType);
			setTriggerFlags(L0TPData->l0TriggerFlags);
			return L0TPData->l0TriggerType;
		}
		else {
			LOG_ERROR("Corrupted event! Could not retrieve the L0TP information!!");
		}
	}
	return 1;
}

void Event::updateMissingEventsStats() {

	for (int sourceNum = SourceIDManager::NUMBER_OF_L0_DATA_SOURCES - 1;
			sourceNum >= 0; sourceNum--) {
		l0::Subevent* subevent = getL0SubeventBySourceIDNum(sourceNum);
		if (subevent->getNumberOfFragments()
				!= subevent->getNumberOfExpectedFragments()) {
			MissingEventsBySourceNum_[sourceNum].fetch_add(1,
					std::memory_order_relaxed);
//#ifdef USE_ERS
//			ers::warning(MissingFragments(ERS_HERE, this->getEventNumber(), subevent->getNumberOfExpectedFragments() - subevent->getNumberOfFragments(),
//							subevent->getNumberOfExpectedFragments(), SourceIDManager::sourceIdToDetectorName(SourceIDManager::sourceNumToID(sourceNum))));
//#endif
		}
		int DetId = (int) (SourceIDManager::sourceNumToID(sourceNum));
		for (int ifrag = 0; ifrag < subevent->getNumberOfFragments(); ifrag++) {
			int SubId = (int) (subevent->getFragment(ifrag)->getSourceSubID());
			DetectorStatistics::incrementL0stat(DetId, SubId);
		}
	}

	if (L1Processed_) {
		for (int sourceNum = SourceIDManager::NUMBER_OF_L1_DATA_SOURCES - 1;
				sourceNum >= 0; sourceNum--) {
			l1::Subevent* subevent = getL1SubeventBySourceIDNum(sourceNum);
			if (subevent->getNumberOfFragments()
					!= subevent->getNumberOfExpectedFragments()) {
				MissingL1EventsBySourceNum_[sourceNum].fetch_add(1,
						std::memory_order_relaxed);
//#ifdef USE_ERS
//				ers::warning(MissingFragments(ERS_HERE, this->getEventNumber(), subevent->getNumberOfExpectedFragments() - subevent->getNumberOfFragments(),
//								subevent->getNumberOfExpectedFragments(), SourceIDManager::sourceIdToDetectorName(SourceIDManager::l1SourceNumToID(sourceNum))));
//
//#endif
			}
			for (int ifrag = 0; ifrag < subevent->getNumberOfFragments(); ifrag++) {
				int SubId = (int) (subevent->getFragment(ifrag)->getSourceSubID());
				int Crate = (SubId >> 5) & 0x3f;
				int Slot = SubId & 0x1f;
				DetectorStatistics::incrementL1stat(Crate, Slot);
			}
		}
	}
	return;
}

} /* namespace na62 */
