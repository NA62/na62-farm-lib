/*
 * EventCollector.h
 *
 *  Created on: Dec 6, 2011
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#pragma once
#ifndef EVENTBUILDER_H_
#define EVENTBUILDER_H_

#include <cstdint>
#include <vector>
#include <glog/logging.h>

#include "../utils/AExecutable.h"
#include "Event.h"

namespace na62 {
class Event;
class L1TriggerProcessor;
class L2TriggerProcessor;

namespace cream {
class LKREvent;
} /* namespace cream */

namespace l0 {
class MEPEvent;
} /* namespace l0 */
} /* namespace na62 */

namespace zmq {
class socket_t;
} /* namespace zmq */

namespace na62 {

class EventBuilder: public AExecutable {
public:
	EventBuilder();
	virtual ~EventBuilder();
	static void Initialize();

	static inline const std::atomic<uint64_t>* GetL1TriggerStats() const {
		return L1Triggers_;
	}

	static inline const std::atomic<uint64_t>* GetL2TriggerStats() const {
		return L2Triggers_;
	}

private:
	void thread();

	void handleL0Data(l0::MEPEvent * mepEvent);
	void handleLKRData(cream::LKREvent * lkrEvent);

	void processL1(Event *event);
	void processL2(Event * event);

	/*
	 * @return <true> if any packet has been sent (time has passed)
	 */
	void sendL1RequestToCREAMS(Event * event);

	static void SendEOBBroadcast(uint32_t eventNumber,
			uint32_t finishedBurstID);

	inline uint32_t getCurrentBurstID() {
		return threadCurrentBurstID_;
	}

	static void SetNextBurstID(uint32_t nextBurstID) {
		LOG(INFO)<<"Changing BurstID to " << nextBurstID;
		for (unsigned int i = 0; i < Instances_.size(); i++) {
			Instances_[i]->setNextBurstID(nextBurstID);
		}
	}

	/*
	 * The burst ID will be changed as soon as the next L0 element is received. This way we can still receive CREAM data.
	 */
	inline void setNextBurstID(uint32_t nextBurstID) {
		changeBurstID_ = true;
		nextBurstID_ = nextBurstID;
	}

	zmq::socket_t* L0Socket_;
	zmq::socket_t* LKrSocket_;

	std::vector<Event*> eventPool;

	const int NUMBER_OF_EBS;

	bool changeBurstID_;
	uint32_t nextBurstID_;
	uint32_t threadCurrentBurstID_;

	L1TriggerProcessor* L1processor_;
	L2TriggerProcessor* L2processor_;

	static std::vector<EventBuilder*> Instances_;

	static std::atomic<uint64_t>* L1Triggers_;
	static std::atomic<uint64_t>* L2Triggers_;

};

}
/* namespace na62 */
#endif /* EVENTBUILDER_H_ */
