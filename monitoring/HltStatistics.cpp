/*
 * HltStatistics.cpp
 *
 *  Created on: Oct 13, 2016
 *      Author: root
 */

#include <monitoring/HltStatistics.h>
//#include <l1/L1TriggerProcessor.h>

namespace na62 {

std::atomic<uint64_t> HltStatistics::L1InputEvents_(0);
std::atomic<uint64_t> HltStatistics::L1PhysicsEvents_(0);
std::atomic<uint64_t> HltStatistics::L1PhysicsEventsByMultipleMasks_(0);
std::atomic<uint64_t>* HltStatistics::L1Triggers_ = new std::atomic<uint64_t>[0xFF + 1];
std::atomic<uint64_t> HltStatistics::L1InputEventsPerBurst_(0);

HltStatistics::HltStatistics() {
	// TODO Auto-generated constructor stub

}
void HltStatistics::initialize() {
	for (int i = 0; i != 0xFF + 1; i++) {
		L1Triggers_[i] = 0;
	}
}

void HltStatistics::updateStatistics(Event* event, uint_fast8_t l1TriggerTypeWord) {
	HltStatistics::SumL1InputEvents(1);
	//LOG_ERROR("HLT Updating Statistics: " << HltStatistics::GetL1InputEvents() << " /" <<  L1TriggerProcessor::GetL1InputStats());
	HltStatistics::SumL1InputEventsPerBurst(1);
	if (l1TriggerTypeWord != 0) {
		HltStatistics::SumL1TriggerStats(1, l1TriggerTypeWord);
	} else {
		//event has been reduced or downscaled
	}

	//This counter should not be incremented if the event has been marked as downscaled
	if (event->isPhysicsTriggerEvent()) {
		HltStatistics::SumL1PhysicsStats(1);
		if (__builtin_popcount((uint) event->getTriggerFlags()) > 1) {
			HltStatistics::SumL1PhysicsByMultipleMasksStats(1);
		}
	}
}


HltStatistics::~HltStatistics() {
	// TODO Auto-generated destructor stub
}

} /* namespace na62 */
