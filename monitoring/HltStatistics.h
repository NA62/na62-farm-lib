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
	static void updateL1Statistics(Event* event, uint_fast8_t l1TriggerTypeWord);
	static void updateL2Statistics(Event* event, uint_fast8_t l2Trigger);
	static void updateStorageStatistics();

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

	//Functions to manipulate the maps
	static inline uint64_t SumCounter(std::string key, uint amount) {
		return cumulativeCounters_[key].fetch_add(amount, std::memory_order_relaxed);
	}
	static inline uint64_t GetCounter(std::string key) {
			return cumulativeCounters_[key];
	}

	static void CountersSnapshot() {
		//coping all values in the snapshot map
		for (auto& key: ExtractKeys()) {
			cumulativeCountersSnapshot_[key] = GetCounter(key);
		}
	}

	static uint64_t GetLastBurstCounter(std::string key) {
		return cumulativeCounters_[key] - cumulativeCountersSnapshot_[key];
	}

	static std::vector<std::string> ExtractKeys() {
		std::vector<std::string> keys;
		for (auto const& counter : cumulativeCounters_) {
			keys.push_back(counter.first);
		}
	  return keys;
	}

private:
	static std::atomic<uint64_t> L1InputEvents_;
	static std::atomic<uint64_t> L1PhysicsEvents_;
	static std::atomic<uint64_t> L1PhysicsEventsByMultipleMasks_;
	static std::atomic<uint64_t>* L1Triggers_;
	static std::atomic<uint64_t> L1InputEventsPerBurst_;

	//Map containing counters countinuously updated by the farm
	static std::map<std::string, std::atomic<uint64_t>> cumulativeCounters_;
	//Snapshot of counters at the last eob commands
	static std::map<std::string, std::atomic<uint64_t>> cumulativeCountersSnapshot_;

};





} /* namespace na62 */

#endif /* MONITORING_HLTSTATISTICS_H_ */
