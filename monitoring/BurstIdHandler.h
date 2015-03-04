/*
 * BurstIdHandler.h
 *
 *  Created on: May 27, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef BURSTIDHANDLER_H_
#define BURSTIDHANDLER_H_

#include <boost/timer/timer.hpp>
#include <cstdint>
#include <iostream>

#include "../options/Logging.h"

namespace na62 {

class BurstIdHandler {
public:
	BurstIdHandler();
	virtual ~BurstIdHandler();

	static void setNextBurstID(uint32_t nextBurstID) {
		nextBurstId_ = nextBurstID;
		EOBReceivedTimer_.start();
		LOG_INFO<<"Changing BurstID to " << nextBurstID << ENDL;
	}

	static uint32_t getCurrentBurstId() {
		return currentBurstID_;
	}

	static uint32_t getNextBurstId() {
		return nextBurstId_;
	}

	static long int getTimeSinceLastEOB() {
		return EOBReceivedTimer_.elapsed().wall;
	}

	static inline bool isInBurst() {
		return nextBurstId_ == currentBurstID_;
	}

	static void checkBurstIdChange() {
		if (nextBurstId_ != currentBurstID_
				&& EOBReceivedTimer_.elapsed().wall / 1E6 > 1000 /*1s*/) {
			currentBurstID_ = nextBurstId_;
		}
	}

	static void initialize(uint startBurstID) {
		currentBurstID_ = startBurstID;
		nextBurstId_ = currentBurstID_;
	}

private:
	static boost::timer::cpu_timer EOBReceivedTimer_;

	/*
	 * Store the current Burst ID and the next one separately. As soon as an EOB event is
	 * received the nextBurstID_ will be set. Then the currentBurstID will be updated later
	 * to make sure currently enqueued frames in other threads are not processed with
	 * the new burstID
	 */
	static uint nextBurstId_;
	static uint currentBurstID_;
};

}
/* namespace na62 */

#endif /* BURSTIDHANDLER_H_ */
