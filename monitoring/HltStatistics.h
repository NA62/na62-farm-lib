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
#include <structs/Event.h>

namespace na62 {

class HltStatistics {
public:
	HltStatistics();
	virtual ~HltStatistics();
	static void initialize();
	static void updateL1Statistics(Event* const event, uint_fast8_t l1Trigger);
	static void updateL2Statistics(Event* event, uint_fast8_t l2Trigger);
	static void updateStorageStatistics();

	//TODO remove
	static inline std::atomic<uint64_t>* getL1TriggerStats() {
		return L1Triggers_;
	}
	static inline std::atomic<uint64_t>* sumL1TriggerStats(int amount, uint_fast8_t l1Trigger) {
		L1Triggers_[l1Trigger].fetch_add(amount, std::memory_order_relaxed);
		return L1Triggers_;
	}

	//Functions to manipulate the maps
	static inline uint64_t sumCounter(std::string key, uint amount) {
		return counters_[key].fetch_add(amount, std::memory_order_relaxed);
	}
	static inline uint64_t getCounter(std::string key) {
			return counters_[key];
	}

	//Functions to manipulate the maps for multidimensional counter
	static inline uint64_t sumDimensionalCounter(std::string key, uint array_index, uint amount) {
		return dimensionalCounters_[key][array_index].fetch_add(amount, std::memory_order_relaxed);
	}
	static inline uint64_t getDimensionalCounter(std::string key, uint array_index) {
			return dimensionalCounters_[key][array_index];
	}

	static void countersReset() {
		for (auto& key: extractKeys()) {
			counters_[key] = 0;
		}
	}

	static uint64_t getRollingCounter(std::string key) {
		return counters_[key];
	}

	static std::string serializeDimensionalCounter(std::string key) {
		std::stringstream serializedConters;
		for(uint index=0; index<16; index++) {
			if(dimensionalCounters_[key][index] > 0) {//Zero suppression
				serializedConters << index << ":" <<dimensionalCounters_[key][index] << ";";
 			}
		}
		return serializedConters.str();
	}

	static void printCounter(){
		for (auto const& counter : counters_) {
			    std::cout << counter.first << " => " << counter.second << '\n';
		}
	}
	static void printDimensionalCounter(){
		for (auto const& counter : dimensionalCounters_) {
			for(uint i=0; i<16; i++)
				std::cout << counter.first << " => " << counter.second[i] << '\n';
		}
	}

	static std::vector<std::string> extractKeys() {
		std::vector<std::string> keys;
		for (auto const& counter : counters_) {
			keys.push_back(counter.first);
		}
		return keys;
	}

	static std::vector<std::string> extractDimensionalKeys() {
		std::vector<std::string> keys;
		for (auto const& counter : dimensionalCounters_) {
			keys.push_back(counter.first);
		}
		return keys;
	}

private:
	static std::atomic<uint64_t>* L1Triggers_;

	//Map containing counters continuously updated by the farm
	static std::map<std::string, std::atomic<uint64_t>> counters_;
	static std::map<std::string, std::array<std::atomic<uint64_t>, 16>> dimensionalCounters_;
};

} /* namespace na62 */

#endif /* MONITORING_HLTSTATISTICS_H_ */
