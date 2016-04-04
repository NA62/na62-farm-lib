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
#include <mutex>
#include <thread>
#include <functional>
#include "../utils/AExecutable.h"
#include "FarmStatistics.h"

#include "../options/Logging.h"
#include "../options/Options.h"

namespace na62 {

class BurstIdHandler: public AExecutable {
public:

	static void setNextBurstID(uint_fast32_t nextBurstID) {

		std::lock_guard<std::mutex> lk(timerMutex_);
		//EOBReceivedTimer_.elapsed().clear();
		EOBReceivedTimer_.start();
		nextBurstId_ = nextBurstID;
		LOG_INFO<<"Changing BurstID to " << nextBurstID << ENDL;
		//resetCounter_=true;
	}

	static uint_fast32_t getCurrentBurstId() {
		return currentBurstID_;
	}

	static long int getTimeSinceLastEOB() {
		std::lock_guard<std::mutex> lk(timerMutex_);
		return (EOBReceivedTimer_.elapsed().wall / 1E9);
	}

	static inline bool isInBurst() {
		return nextBurstId_ == currentBurstID_;
	}

	static inline bool flushBurst() {
		return flushBurst_ ;
	}

	static void initialize(uint startBurstID, std::function<void()> burstCleanupFunction) {
		currentBurstID_ = startBurstID;
		nextBurstId_ = currentBurstID_;
		running_ = true;
		flushBurst_ = false;
		burstCleanupFunction_ = burstCleanupFunction;
	}

	static void shutDown() {
		running_=false;
	}
	void thread();

private:
	/**
	 * Method is called every time the last event of a burst has been processed
	 */
	//void onBurstFinished();

	static boost::timer::cpu_timer EOBReceivedTimer_;
	static std::mutex timerMutex_;

	/*
	 * Store the current Burst ID and the next one separately. As soon as an EOB event is
	 * received the nextBurstID_ will be set. Then the currentBurstID will be updated later
	 * to make sure currently enqueued frames in other threads are not processed with
	 * the new burstID
	 */
	static uint nextBurstId_;
	static uint currentBurstID_;
	static std::atomic<bool> running_;
	static std::atomic<bool> flushBurst_;
	static std::function<void()> burstCleanupFunction_;
};

}
/* namespace na62 */

#endif /* BURSTIDHANDLER_H_ */
