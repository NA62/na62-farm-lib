/*
 * HltStatistics.h
 *
 *  Created on: Oct 13, 2016
 *      Author: root
 */

#ifndef MONITORING_HLTSTATISTICS_H_
#define MONITORING_HLTSTATISTICS_H_

#include <atomic>
#include <eventBuilding/Event.h>

namespace na62 {


class HltStatistics {
public:
	HltStatistics();
	virtual ~HltStatistics();
	static void initialize();
	static void updateStatistics(Event* event, uint_fast8_t l1TriggerTypeWord);

	static inline uint64_t GetL1InputEvents() {
		return L1InputEvents_;
	}
	static inline uint64_t SumL1InputEvents(int amount) {
		return L1InputEvents_.fetch_add(amount, std::memory_order_relaxed);
	}

	static inline uint64_t GetL1PhysicsStats() {
		return L1PhysicsEvents_;
	}
	static inline uint64_t SumL1PhysicsStats(int amount) {
		return L1PhysicsEvents_.fetch_add(amount, std::memory_order_relaxed);
	}

	static inline uint64_t GetL1PhysicsByMultipleMasksStats() {
		return L1PhysicsEventsByMultipleMasks_;
	}
	static inline uint64_t SumL1PhysicsByMultipleMasksStats(int amount) {
		return L1PhysicsEventsByMultipleMasks_.fetch_add(amount, std::memory_order_relaxed);
	}

	static inline std::atomic<uint64_t>* GetL1TriggerStats() {
		return L1Triggers_;
	}
	static inline std::atomic<uint64_t>* SumL1TriggerStats(int amount, uint_fast8_t l1Trigger) {
		L1Triggers_[l1Trigger].fetch_add(amount, std::memory_order_relaxed);
		return L1Triggers_;
	}

	static inline uint64_t GetL1InputEventsPerBurst() {
		return L1InputEventsPerBurst_;
	}
	static inline uint64_t SumL1InputEventsPerBurst(int amount) {
		return L1InputEventsPerBurst_.fetch_add(amount, std::memory_order_relaxed);
	}
	static void ResetL1InputEventsPerBurst() {
		L1InputEventsPerBurst_ = 0;
	}

private:
	static std::atomic<uint64_t> L1InputEvents_;
	static std::atomic<uint64_t> L1PhysicsEvents_;
	static std::atomic<uint64_t> L1PhysicsEventsByMultipleMasks_;
	static std::atomic<uint64_t>* L1Triggers_;
	static std::atomic<uint64_t> L1InputEventsPerBurst_;

};





} /* namespace na62 */

#endif /* MONITORING_HLTSTATISTICS_H_ */
