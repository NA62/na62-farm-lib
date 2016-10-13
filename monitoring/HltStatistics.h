/*
 * HltStatistics.h
 *
 *  Created on: Oct 13, 2016
 *      Author: root
 */

#ifndef MONITORING_HLTSTATISTICS_H_
#define MONITORING_HLTSTATISTICS_H_

namespace na62 {

#include <atomic>


class HltStatistics {
public:
	HltStatistics();
	virtual ~HltStatistics();

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
		return L1InputEvents_.fetch_add(amount, std::memory_order_relaxed);
	}

private:
	static std::atomic<uint64_t> L1InputEvents_;
	static std::atomic<uint64_t> L1PhysicsEvents_;

};





} /* namespace na62 */

#endif /* MONITORING_HLTSTATISTICS_H_ */
