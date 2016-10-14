/*
 * HltStatistics.cpp
 *
 *  Created on: Oct 13, 2016
 *      Author: root
 */

#include <monitoring/HltStatistics.h>

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


HltStatistics::~HltStatistics() {
	// TODO Auto-generated destructor stub
}

} /* namespace na62 */
