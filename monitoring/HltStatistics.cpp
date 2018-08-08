/*
 * HltStatistics.cpp
 *
 *  Created on: Oct 13, 2016
 *      Author: root
 */

#include <monitoring/HltStatistics.h>
#include <monitoring/BurstIdHandler.h>
#include <array>

namespace na62 {

std::atomic<uint64_t>* HltStatistics::L1Triggers_ = new std::atomic<uint64_t>[0xFF + 1];
std::atomic<uint64_t>* HltStatistics::L2Triggers_ = new std::atomic<uint64_t>[0xFF + 1];

std::map<std::string, std::atomic<uint64_t>> HltStatistics::counters_;
std::map<std::string, std::array<std::atomic<uint64_t>, 16>> HltStatistics::dimensionalCounters_;
l1EOBInfo HltStatistics::l1EobStruct_;
l2EOBInfo HltStatistics::l2EobStruct_;
int HltStatistics::logicalID_ = 0;

HltStatistics::HltStatistics() {
	// TODO Auto-generated constructor stub
}

void HltStatistics::initialize(int logicalID) {

	logicalID_ = logicalID;

	counters_["L1InputEvents"] = 0;
	counters_["L1SpecialEvents"] = 0;
	counters_["L1ControlEvents"] = 0;
	counters_["L1PeriodicsEvents"] = 0;
	counters_["L1PhysicsEvents"] = 0;
	counters_["L1PhysicsEventsByMultipleMasks"] = 0;
	counters_["L1RequestToCreams"] = 0;
	counters_["L1OutputEvents"] = 0;
	counters_["L1AcceptedEvents"] = 0;
	counters_["L1TimeoutEvents"] = 0;
	counters_["L1AllDisabledEvents"] = 0;
	counters_["L1BypassEvents"] = 0;
	counters_["L1FlagAlgoEvents"] = 0;
	counters_["L1AutoPassEvents"] = 0;
	counters_["L1CorruptedHeader"] = 0;

	counters_["L2InputEvents"] = 0;
	counters_["L2SpecialEvents"] = 0;
	counters_["L2ControlEvents"] = 0;
	counters_["L2PeriodicsEvents"] = 0;
	counters_["L2PhysicsEvents"] = 0;
	counters_["L2PhysicsEventsByMultipleMasks"] = 0;
	counters_["L2OutputEvents"] = 0;
	counters_["L2AcceptedEvents"] = 0;
	counters_["L2TimeoutEvents"] = 0;
	counters_["L2AllDisabledEvents"] = 0;
	counters_["L2BypassEvents"] = 0;
	counters_["L2FlagAlgoEvents"] = 0;
	counters_["L2AutoPassEvents"] = 0;

	for (int i = 0; i != 0xFF + 1; i++) {
		L1Triggers_[i] = 0;
		L2Triggers_[i] = 0;
	}

	for (int i = 0; i != 16; ++i) {
		dimensionalCounters_["L1InputEventsPerMask"][i] = 0;
		dimensionalCounters_["L1AcceptedEventsPerMask"][i] = 0;
		dimensionalCounters_["L2InputEventsPerMask"][i] = 0;
		dimensionalCounters_["L2AcceptedEventsPerMask"][i] = 0;
	}
}

void HltStatistics::updateL1Statistics(Event* const event, uint_fast8_t l1Trigger) {
	/*
	 * Method for stats update - all L1 monitoring counters but L1RequestToCreams are incremented here
	 */
	uint_fast16_t l0TrigFlags = event->getTriggerFlags();
	HltStatistics::sumCounter("L1InputEvents", 1);
	//LOG_INFO("Update L1 Statistics: L1InputsEvents: " << HltStatistics::getCounter("L1InputEvents"));

	/*
	 *Special triggers are all counted together
	 *l0 trigger words (NZS): SOB=0x22, EOB=0x23, PEDESTAL_LKR=0x2d, CALIBRATION_LKR(1,2,3,4)=0x30,31,32,33
	 *l0 trigger word (ZS): GTK pulsers =0x2c
	 *Separate treatments in processing due to different requests for zero-suppression in LKr
	 */
	if (event->isSpecialTriggerEvent() || event->isPulserGTKTriggerEvent()) {
		HltStatistics::sumCounter("L1SpecialEvents", 1);
	}

	if (event->isControlTriggerEvent()) {
		HltStatistics::sumCounter("L1ControlEvents", 1);
	}
	if (event->isPeriodicTriggerEvent()) {
		HltStatistics::sumCounter("L1PeriodicsEvents", 1);
	}
	if (event->isPhysicsTriggerEvent()) {
		HltStatistics::sumCounter("L1PhysicsEvents", 1);
		if (__builtin_popcount((uint) l0TrigFlags) > 1)
			HltStatistics::sumCounter("L1PhysicsEventsByMultipleMasks", 1);
		for (int i = 0; i != 16; i++) {
			if (l0TrigFlags & (1 << i)) {
				HltStatistics::sumDimensionalCounter("L1InputEventsPerMask", i, 1);
				if (event->getL1TriggerWord(i)) {
					HltStatistics::sumDimensionalCounter("L1AcceptedEventsPerMask", i, 1);
				}
			}
		}
	}

	/*
	 * l1Trigger 8-bit word contains event based l1 trigger info
	 * bit 0 = physics verdict OR of all l1 trigger masks
	 * bit 3 = timeout flag (error in trigger configuration)
	 * bit 4 = l1 trigger disable on this mask
	 * bit 5 = bypass event (control, periodics, special triggers)
	 * bit 6 = at least one algo being processed in flagging
	 * bit 7 = AutoPass (AP) event (fraction of overall bandwidth)
	 */
	if (l1Trigger & TRIGGER_L1_PHYSICS) {
		HltStatistics::sumCounter("L1AcceptedEvents", 1);
	}
	if (l1Trigger & TRIGGER_L1_TIMEOUT) {
		HltStatistics::sumCounter("L1TimeoutEvents", 1);
	}
	if (l1Trigger & TRIGGER_L1_ALLDISABLED) {
		HltStatistics::sumCounter("L1AllDisabledEvents", 1);
	}
	if (l1Trigger & TRIGGER_L1_BYPASS) {
		HltStatistics::sumCounter("L1BypassEvents", 1);
	}
	if (l1Trigger & TRIGGER_L1_FLAGALGO) {
		HltStatistics::sumCounter("L1FlagAlgoEvents", 1);
	}
	if (l1Trigger & TRIGGER_L1_AUTOPASS) {
		HltStatistics::sumCounter("L1AutoPassEvents", 1);
	}
	if (l1Trigger != 0) {
		HltStatistics::sumCounter("L1OutputEvents", 1);
		HltStatistics::sumL1TriggerStats(1, l1Trigger);
	} else {
		//event has been reduced or downscaled
	}
}

void HltStatistics::updateL2Statistics(Event* const event, uint_fast8_t l2Trigger) {
	/*
	 * Method for stats update - all L1 monitoring counters but L1RequestToCreams are incremented here
	 */
	uint_fast16_t l0TrigFlags = event->getTriggerFlags();
	HltStatistics::sumCounter("L2InputEvents", 1);

	/*
	 *Special triggers are all counted together
	 *l0 trigger words (NZS): SOB=0x22, EOB=0x23, PEDESTAL_LKR=0x2d, CALIBRATION_LKR(1,2,3,4)=0x30,31,32,33
	 *l0 trigger word (ZS): GTK pulsers =0x2c
	 */
	if (event->isSpecialTriggerEvent() || event->isPulserGTKTriggerEvent()) {
		HltStatistics::sumCounter("L2SpecialEvents", 1);
	}
	if (event->isControlTriggerEvent()) {
		HltStatistics::sumCounter("L2ControlEvents", 1);
	}
	if (event->isPeriodicTriggerEvent()) {
		HltStatistics::sumCounter("L2PeriodicsEvents", 1);
	}
	if (event->isPhysicsTriggerEvent()) {
		HltStatistics::sumCounter("L2PhysicsEvents", 1);
		if (__builtin_popcount((uint) l0TrigFlags) > 1)
			HltStatistics::sumCounter("L2PhysicsEventsByMultipleMasks", 1);
		for (int i = 0; i != 16; i++) {
			if (l0TrigFlags & (1 << i)) {
				HltStatistics::sumDimensionalCounter("L2InputEventsPerMask", i, 1);
				if (event->getL2TriggerWord(i)) {
					HltStatistics::sumDimensionalCounter("L2AcceptedEventsPerMask", i, 1);
				}
			}
		}
	}

	/*
	 * l2Trigger 8-bit word contains event based l1 trigger info
	 * bit 0 = physics verdict OR of all l1 trigger masks
	 * bit 4 = l1 trigger disable on this mask
	 * bit 5 = bypass event (control, periodics, special triggers)
	 * bit 6 = at least one algo being processed in flagging
	 * bit 7 = AutoPass (AP) event (fraction of overall bandwidth)
	 */
	if (l2Trigger & TRIGGER_L2_PHYSICS) {
		HltStatistics::sumCounter("L2AcceptedEvents", 1);
	}
	if (l2Trigger & TRIGGER_L2_TIMEOUT) {
		HltStatistics::sumCounter("L2TimeoutEvents", 1);
	}
	if (l2Trigger & TRIGGER_L2_ALLDISABLED) {
		HltStatistics::sumCounter("L2AllDisabledEvents", 1);
	}
	if (l2Trigger & TRIGGER_L2_BYPASS) {
		HltStatistics::sumCounter("L2BypassEvents", 1);
	}
	if (l2Trigger & TRIGGER_L2_FLAGALGO) {
		HltStatistics::sumCounter("L2FlagAlgoEvents", 1);
	}
	if (l2Trigger & TRIGGER_L2_AUTOPASS) {
		HltStatistics::sumCounter("L2AutoPassEvents", 1);
	}
	if (l2Trigger != 0) {
		HltStatistics::sumCounter("L2OutputEvents", 1);
		HltStatistics::sumL2TriggerStats(1, l2Trigger);
	} else {
		//event has been reduced or downscaled
	}
}

void HltStatistics::updateStorageStatistics(uint64_t BytesSentToStorage) {
	HltStatistics::sumCounter("EventsToMerger", 1);
	HltStatistics::sumCounter("BytesToMerger", BytesSentToStorage);
}

std::string HltStatistics::fillL1Eob() {

	/*
	 * Prepare header - this is the same as for each detector source - do not change it!)
	 */
	l1EobStruct_.header.blockID = logicalID_;
	l1EobStruct_.header.length = sizeof(l1EOBInfo) / 4;
	l1EobStruct_.header.detectorID = SOURCE_ID_L1;
	l1EobStruct_.header.eobTimestamp = BurstIdHandler::getEOBTime();

	/*
	 * Fill EOB data - A change here will affect the event data format.
	 * Any modifications must be flagged with a different data format !!!
	 */

	l1EobStruct_.l1EobData.formatVersion = 0;
	l1EobStruct_.l1EobData.timeoutFlag = (getCounter("L1TimeoutEvents") > 0);
	l1EobStruct_.l1EobData.reserved = 0;

	for (auto const& counter : counters_) {
		if (counter.first == "L1CorruptedHeader") {
			l1EobStruct_.l1EobData.extraReserved = counter.second;
		}
		if (counter.first == "L1InputEvents") {
			l1EobStruct_.l1EobData.L1InputEvents = counter.second;
		}
		if (counter.first == "L1SpecialEvents") {
			l1EobStruct_.l1EobData.L1SpecialEvents = counter.second;
		}
		if (counter.first == "L1ControlEvents") {
			l1EobStruct_.l1EobData.L1ControlEvents = counter.second;
		}
		if (counter.first == "L1PeriodicsEvents") {
			l1EobStruct_.l1EobData.L1PeriodicsEvents = counter.second;
		}
		if (counter.first == "L1PhysicsEvents") {
			l1EobStruct_.l1EobData.L1PhysicsEvents = counter.second;
		}
		if (counter.first == "L1PhysicsEventsByMultipleMasks") {
			l1EobStruct_.l1EobData.L1PhysicsEventsByMultipleMasks = counter.second;
		}
		if (counter.first == "L1RequestToCreams") {
			l1EobStruct_.l1EobData.L1RequestToCreams = counter.second;
		}
		if (counter.first == "L1OutputEvents") {
			l1EobStruct_.l1EobData.L1OutputEvents = counter.second;
		}
		if (counter.first == "L1AcceptedEvents") {
			l1EobStruct_.l1EobData.L1AcceptedEvents = counter.second;
		}
		if (counter.first == "L1TimeoutEvents") {
			l1EobStruct_.l1EobData.L1TimeoutEvents = counter.second;
		}
		if (counter.first == "L1AllDisabledEvents") {
			l1EobStruct_.l1EobData.L1AllDisabledEvents = counter.second;
		}
		if (counter.first == "L1BypassEvents") {
			l1EobStruct_.l1EobData.L1BypassEvents = counter.second;
		}
		if (counter.first == "L1FlagAlgoEvents") {
			l1EobStruct_.l1EobData.L1FlagAlgoEvents = counter.second;
		}
		if (counter.first == "L1AutoPassEvents") {
			l1EobStruct_.l1EobData.L1AutoPassEvents = counter.second;
		}
	}
	for (auto const& dimCounter : dimensionalCounters_) {
		for (uint i = 0; i < 16; i++) {
			if (dimCounter.first == "L1InputEventsPerMask") {
				l1EobStruct_.l1EobData.l1Mask[i].L1InputEventsPerMask = dimCounter.second[i];
			}
			if (dimCounter.first == "L1AcceptedEventsPerMask") {
				l1EobStruct_.l1EobData.l1Mask[i].L1AcceptedEventsPerMask = dimCounter.second[i];
			}
			l1EobStruct_.l1EobData.l1Mask[i].L1ReservedPerMask = 0;
		}
	}
	char serializedStruct [sizeof(l1EOBInfo)];
    memcpy((void*) &serializedStruct, (void*) &l1EobStruct_, sizeof(l1EOBInfo));
    return std::string(serializedStruct, sizeof(l1EOBInfo));
}

std::string HltStatistics::fillL2Eob() {
	/*
	 * Prepare header - this is the same as for each detector source - do not change it!)
	 */
	l2EobStruct_.header.blockID = logicalID_;
	l2EobStruct_.header.length = sizeof(l2EOBInfo) / 4;
	l2EobStruct_.header.detectorID = SOURCE_ID_L2;
	l2EobStruct_.header.eobTimestamp = BurstIdHandler::getEOBTime();

	/*
	 * Fill EOB data - A change here will affect the event data format.
	 * Any modifications must be flagged with a different data format !!!
	 */

	l2EobStruct_.l2EobData.formatVersion = 0;
	l2EobStruct_.l2EobData.timeoutFlag = (getCounter("L2TimeoutEvents") > 0);
	l2EobStruct_.l2EobData.reserved = 0;
	l2EobStruct_.l2EobData.extraReserved = 0;

	for (auto const& counter : counters_) {
		if (counter.first == "L2InputEvents") {
			l2EobStruct_.l2EobData.L2InputEvents = counter.second;
		}
		if (counter.first == "L2SpecialEvents") {
			l2EobStruct_.l2EobData.L2SpecialEvents = counter.second;
		}
		if (counter.first == "L2ControlEvents") {
			l2EobStruct_.l2EobData.L2ControlEvents = counter.second;
		}
		if (counter.first == "L2PeriodicsEvents") {
			l2EobStruct_.l2EobData.L2PeriodicsEvents = counter.second;
		}
		if (counter.first == "L2PhysicsEvents") {
			l2EobStruct_.l2EobData.L2PhysicsEvents = counter.second;
		}
		if (counter.first == "L2PhysicsEventsByMultipleMasks") {
			l2EobStruct_.l2EobData.L2PhysicsEventsByMultipleMasks = counter.second;
		}
		if (counter.first == "L2OutputEvents") {
			l2EobStruct_.l2EobData.L2OutputEvents = counter.second;
		}
		if (counter.first == "L2AcceptedEvents") {
			l2EobStruct_.l2EobData.L2AcceptedEvents = counter.second;
		}
		if (counter.first == "L2TimeoutEvents") {
			l2EobStruct_.l2EobData.L2TimeoutEvents = counter.second;
		}
		if (counter.first == "L2AllDisabledEvents") {
			l2EobStruct_.l2EobData.L2AllDisabledEvents = counter.second;
		}
		if (counter.first == "L2BypassEvents") {
			l2EobStruct_.l2EobData.L2BypassEvents = counter.second;
		}
		if (counter.first == "L2FlagAlgoEvents") {
			l2EobStruct_.l2EobData.L2FlagAlgoEvents = counter.second;
		}
		if (counter.first == "L2AutoPassEvents") {
			l2EobStruct_.l2EobData.L2AutoPassEvents = counter.second;
		}
	}
	for (auto const& dimCounter : dimensionalCounters_) {
		for (uint i = 0; i < 16; i++) {
			if (dimCounter.first == "L2InputEventsPerMask") {
				l2EobStruct_.l2EobData.l2Mask[i].L2InputEventsPerMask = dimCounter.second[i];
			}
			if (dimCounter.first == "L2AcceptedEventsPerMask") {
				l2EobStruct_.l2EobData.l2Mask[i].L2AcceptedEventsPerMask = dimCounter.second[i];
			}
			l2EobStruct_.l2EobData.l2Mask[i].L2ReservedPerMask = 0;
		}
	}
	char serializedStruct [sizeof(l2EOBInfo)];
    memcpy((void*) &serializedStruct, (void*) &l2EobStruct_, sizeof(l2EOBInfo));
    return std::string(serializedStruct, sizeof(l2EOBInfo));
}

HltStatistics::~HltStatistics() {
	// TODO Auto-generated destructor stub
}

} /* namespace na62 */
