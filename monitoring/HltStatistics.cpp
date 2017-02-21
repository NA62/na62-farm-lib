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

std::map<std::string, std::atomic<uint64_t>> HltStatistics::cumulativeCounters_;
std::map<std::string, std::atomic<uint64_t>> HltStatistics::cumulativeCountersSnapshot_;
std::map<std::string, std::array<std::atomic<uint64_t>, 16>> HltStatistics::cumulativeDimensionalCounters_;
//std::map<std::string, std::atomic<uint64_t>*> HltStatistics::cumulativeDimensionalCounters_;


HltStatistics::HltStatistics() {
	// TODO Auto-generated constructor stub

}
void HltStatistics::initialize() {

	cumulativeCounters_["L1InputEvents_"] = 0;

	cumulativeCounters_["L1SpecialEvents_"] = 0;
	cumulativeCounters_["L1ControlEvents_"] = 0;
	cumulativeCounters_["L1PeriodicsEvents_"] = 0;
	cumulativeCounters_["L1PhysicsEvents_"] = 0;
	cumulativeCounters_["L1PhysicsEventsByMultipleMasks_"] = 0;

//	cumulativeDimensionalCounters_["L1InputEventsPerMask_"] = new std::atomic<uint64_t>[16];

//	cumulativeDimensionalCounters_["L1AcceptedEventsPerMask_"] = new std::atomic<uint64_t>[16];

	cumulativeCounters_["L1OutputEvents_"] = 0;

	cumulativeCounters_["L1AcceptedEvents_"] = 0;
	cumulativeCounters_["L1TimeoutEvents_"] = 0;
	cumulativeCounters_["L1AllDisabledEvents_"] = 0;
	cumulativeCounters_["L1BypassEvents_"] = 0;
	cumulativeCounters_["L1FlagAlgoEvents_"] = 0;
	cumulativeCounters_["L1AutoPassEvents_"] = 0;


	for (int i = 0; i != 0xFF + 1; i++) {
		L1Triggers_[i] = 0;
	}
	for (int i = 0; i != 16; ++i) {
		cumulativeDimensionalCounters_["L1InputEventsPerMask_"][i] = 0;
		cumulativeDimensionalCounters_["L1AcceptedEventsPerMask_"][i] = 0;
	}
}

void HltStatistics::updateL1Statistics(Event* const event, std::array<uint_fast8_t, 16> l1TriggerWords, uint_fast8_t l1Trigger) {
	HltStatistics::sumCounter("L1InputEvents_", 1);


	uint_fast16_t l0TrigFlags = event->getTriggerFlags();

	/*
	 *Special triggers are all counted together
	 *l0 trigger words (NZS): SOB=0x22, EOB=0x23, PEDESTAL_LKR=0x2d, CALIBRATION_LKR(1,2,3,4)=0x30,31,32,33
	 *l0 trigger word (ZS): GTK pulsers =0x2c
	 *Separate treatments in processing due to different requests for zero-suppression in LKr
	 */
	if (event->isSpecialTriggerEvent() || event->isPulserGTKTriggerEvent()) {
		HltStatistics::sumCounter("L1SpecialEvents_", 1);
	}
	if (event->isControlTriggerEvent()) {
		HltStatistics::sumCounter("L1ControlEvents_", 1);
	}
	if (event->isPeriodicTriggerEvent()) {
		HltStatistics::sumCounter("L1PeriodicsEvents_", 1);
	}
	if (event->isPhysicsTriggerEvent()) {
		HltStatistics::sumCounter("L1PhysicsEvents_", 1);
		if (__builtin_popcount((uint) l0TrigFlags) > 1)
			HltStatistics::sumCounter("L1PhysicsEventsByMultipleMasks_", 1);
		for (int i = 0; i != 16; i++) {
			if (l0TrigFlags & (1 << i)) {
				LOG_INFO("i " << i << " l0TrigFlags " << std::hex << (uint) l0TrigFlags << std::dec);
				HltStatistics::sumDimensionalCounter("L1InputEventsPerMask_", i, 1);
				if(l1TriggerWords[i]){
					LOG_INFO("i " << i << " l1TrigWrd " << (uint) l1TriggerWords[i]);
					HltStatistics::sumDimensionalCounter("L1AcceptedEventsPerMask_", i, 1);
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

	if (l1Trigger & TRIGGER_L1_PHYSICS) HltStatistics::sumCounter("L1AcceptedEvents_",1);
	if (l1Trigger & TRIGGER_L1_TIMEOUT) HltStatistics::sumCounter("L1TimeoutEvents_",1);
	if (l1Trigger & TRIGGER_L1_ALLDISABLED) HltStatistics::sumCounter("L1AllDisabledEvents_",1);
	if (l1Trigger & TRIGGER_L1_BYPASS) HltStatistics::sumCounter("L1BypassEvents_",1);
	if (l1Trigger & TRIGGER_L1_FLAGALGO) HltStatistics::sumCounter("L1FlagAlgoEvents_",1);
	if (l1Trigger & TRIGGER_L1_AUTOPASS) HltStatistics::sumCounter("L1AutoPassEvents_",1);

	if (l1Trigger != 0) {
		HltStatistics::sumCounter("L1OutputEvents_",1);
		HltStatistics::sumL1TriggerStats(1, l1Trigger);
	} else {
		//event has been reduced or downscaled
	}

//	HltStatistics::printCounter();
//	HltStatistics::printDimensionalCounter();

}

void HltStatistics::updateL2Statistics(Event* event, uint_fast8_t l2Trigger) {
//	HltStatistics::sumCounter("CounterL2", 1);
}

void HltStatistics::updateStorageStatistics() {
//	HltStatistics::sumCounter("CounterStorage", 1);
}

HltStatistics::~HltStatistics() {
	// TODO Auto-generated destructor stub
}

} /* namespace na62 */
