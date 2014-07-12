/*
 * BurstIdHandler.h
 *
 *  Created on: May 27, 2014
 *      Author: root
 */

#ifndef BURSTIDHANDLER_H_
#define BURSTIDHANDLER_H_

#include <boost/timer/timer.hpp>
#ifdef USE_GLOG
#include <glog/logging.h>
#endif
#include <cstdint>
#include <iostream>

namespace na62 {

class BurstIdHandler {
public:
	BurstIdHandler();
	virtual ~BurstIdHandler();

	static void SetNextBurstID(uint32_t nextBurstID) {
		currentBurstID_ = nextBurstID;

		EOBReceivedTimer_.start();
#ifdef USE_GLOG
		LOG(INFO) << "Changing BurstID to " << nextBurstID;
#endif
	}

	static uint32_t getCurrentBurstId() {
		return currentBurstID_;
	}

	static long int getTimeSinceLastEOB() {
		return EOBReceivedTimer_.elapsed().wall;
	}

private:
	static boost::timer::cpu_timer EOBReceivedTimer_;

	static uint32_t currentBurstID_;
};

}
/* namespace na62 */

#endif /* BURSTIDHANDLER_H_ */
