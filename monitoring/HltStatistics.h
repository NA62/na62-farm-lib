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

#define TRIGGER_L1_PHYSICS 0x01
#define TRIGGER_L1_TIMEOUT 0x08
#define TRIGGER_L1_ALLDISABLED 0x10
#define TRIGGER_L1_BYPASS 0x20
#define TRIGGER_L1_FLAGALGO 0x40
#define TRIGGER_L1_AUTOPASS 0x80

namespace na62 {


class HltStatistics {
public:
	HltStatistics();
	virtual ~HltStatistics();
	static void initialize();
	static void updateL1Statistics(Event* const event, std::array<uint_fast8_t, 16> l1TriggerWords, uint_fast8_t l1Trigger);
	static void updateL2Statistics(Event* event, uint_fast8_t l2Trigger);
	static void updateStorageStatistics();

	static inline std::atomic<uint64_t>* getL1TriggerStats() {
		return L1Triggers_;
	}
	static inline std::atomic<uint64_t>* sumL1TriggerStats(int amount, uint_fast8_t l1Trigger) {
		L1Triggers_[l1Trigger].fetch_add(amount, std::memory_order_relaxed);
		return L1Triggers_;
	}

	//Functions to manipulate the maps
	static inline uint64_t sumCounter(std::string key, uint amount) {
		return cumulativeCounters_[key].fetch_add(amount, std::memory_order_relaxed);
	}
	static inline uint64_t getCounter(std::string key) {
			return cumulativeCounters_[key];
	}

	//Functions to manipulate the maps for multidimensinal counter
	static inline uint64_t sumDimensionalCounter(std::string key, uint array_index, uint amount) {
		return cumulativeDimensionalCounters_[key][array_index].fetch_add(amount, std::memory_order_relaxed);
	}
	static inline uint64_t getDimensionalCounter(std::string key, uint array_index) {
			return cumulativeDimensionalCounters_[key][array_index];
	}

	static void countersSnapshot() {
		//coping all values in the snapshot map
		for (auto& key: extractKeys()) {
			cumulativeCountersSnapshot_[key] = getCounter(key);
		}
	}

	static uint64_t getLastBurstCounter(std::string key) {
		return cumulativeCounters_[key] - cumulativeCountersSnapshot_[key];
	}

	static void printCounter(){
		for (auto const& counter : cumulativeCounters_) {
			    std::cout << counter.first << " => " << counter.second << '\n';
		}
	}
	static void printDimensionalCounter(){
		for (auto const& counter : cumulativeDimensionalCounters_) {
			for(uint i=0; i<16; i++)
				std::cout << counter.first << " => " << counter.second[i] << '\n';
		}
	}

	static std::vector<std::string> extractKeys() {
		std::vector<std::string> keys;
		for (auto const& counter : cumulativeCounters_) {
			keys.push_back(counter.first);
		}
	  return keys;
	}

private:
	static std::atomic<uint64_t>* L1Triggers_;

	//Map containing counters continuously updated by the farm
	static std::map<std::string, std::atomic<uint64_t>> cumulativeCounters_;
//	static std::map<std::string, std::atomic<uint64_t>*> cumulativeDimensionalCounters_;
	static std::map<std::string, std::array<std::atomic<uint64_t>, 16>> cumulativeDimensionalCounters_;

	//Snapshot of counters at the last eob commands
	static std::map<std::string, std::atomic<uint64_t>> cumulativeCountersSnapshot_;


};





} /* namespace na62 */

#endif /* MONITORING_HLTSTATISTICS_H_ */
