/*
 * HltStatistics.cpp
 *
 *  Created on: Oct 13, 2016
 *      Author: root
 */

#include <monitoring/HltStatistics.h>
#include <array>

namespace na62 {

std::atomic<uint64_t>* HltStatistics::L1Triggers_ = new std::atomic<uint64_t>[0xFF + 1];
std::atomic<uint64_t>* HltStatistics::L2Triggers_ = new std::atomic<uint64_t>[0xFF + 1];

std::map<std::string, std::atomic<uint64_t>> HltStatistics::counters_;
std::map<std::string, std::array<std::atomic<uint64_t>, 16>> HltStatistics::dimensionalCounters_;

HltStatistics::HltStatistics() {
	// TODO Auto-generated constructor stub
}
void HltStatistics::initialize() {

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
	HltStatistics::sumCounter("L1InputEvents", 1);

	uint_fast16_t l0TrigFlags = event->getTriggerFlags();

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
				if(event->getL1TriggerWord(i)){
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

	if (l1Trigger & TRIGGER_L1_PHYSICS) HltStatistics::sumCounter("L1AcceptedEvents",1);
	if (l1Trigger & TRIGGER_L1_TIMEOUT) HltStatistics::sumCounter("L1TimeoutEvents",1);
	if (l1Trigger & TRIGGER_L1_ALLDISABLED) HltStatistics::sumCounter("L1AllDisabledEvents",1);
	if (l1Trigger & TRIGGER_L1_BYPASS) HltStatistics::sumCounter("L1BypassEvents",1);
	if (l1Trigger & TRIGGER_L1_FLAGALGO) HltStatistics::sumCounter("L1FlagAlgoEvents",1);
	if (l1Trigger & TRIGGER_L1_AUTOPASS) HltStatistics::sumCounter("L1AutoPassEvents",1);

	if (l1Trigger != 0) {
		HltStatistics::sumCounter("L1OutputEvents",1);
		HltStatistics::sumL1TriggerStats(1, l1Trigger);
	} else {
		//event has been reduced or downscaled
	}
//	HltStatistics::printCounter();
//	HltStatistics::printDimensionalCounter();
}

void HltStatistics::updateL2Statistics(Event* const event, uint_fast8_t l2Trigger) {
	HltStatistics::sumCounter("L2InputEvents", 1);

	uint_fast16_t l0TrigFlags = event->getTriggerFlags();

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
				if(event->getL2TriggerWord(i)){
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

	if (l2Trigger & TRIGGER_L2_PHYSICS) HltStatistics::sumCounter("L2AcceptedEvents",1);
	if (l2Trigger & TRIGGER_L2_ALLDISABLED) HltStatistics::sumCounter("L2AllDisabledEvents",1);
	if (l2Trigger & TRIGGER_L2_BYPASS) HltStatistics::sumCounter("L2BypassEvents",1);
	if (l2Trigger & TRIGGER_L2_FLAGALGO) HltStatistics::sumCounter("L2FlagAlgoEvents",1);
	if (l2Trigger & TRIGGER_L2_AUTOPASS) HltStatistics::sumCounter("L2AutoPassEvents",1);

	if (l2Trigger != 0) {
		HltStatistics::sumCounter("L2OutputEvents",1);
		HltStatistics::sumL2TriggerStats(1, l2Trigger);
	} else {
		//event has been reduced or downscaled
	}
//	HltStatistics::printCounter();
//	HltStatistics::printDimensionalCounter();
}

void HltStatistics::updateStorageStatistics() {
//	HltStatistics::sumCounter("CounterStorage", 1);
}

HltStatistics::~HltStatistics() {
	// TODO Auto-generated destructor stub
}

} /* namespace na62 */
