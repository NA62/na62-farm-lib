/*
 * BurstIdHandler.h
 *
 *  Created on: May 27, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef BURSTIDHANDLER_H_
#define BURSTIDHANDLER_H_

#include <boost/timer/timer.hpp>
#include <mutex>
#include <atomic>

#include "../utils/AExecutable.h"
#include "../options/Logging.h"


namespace na62 {

class BurstIdHandler: public AExecutable {
public:

	static void initialize(int flush_burst_millis, int clean_burst_millis) {
		flush_burst_s_ = ((float) flush_burst_millis) / 1000.;
		clean_burst_s_ = ((float) clean_burst_millis) / 1000.;
	}
	static void setNextBurstID(uint_fast32_t nextBurstID) {
		std::lock_guard<std::mutex> lk(timerMutex_);
		//EOBReceivedTimer_.elapsed().clear();
		EOBReceivedTimer_.start();
		nextBurstId_ = nextBurstID;
		LOG_INFO("Changing BurstID to " << nextBurstID);
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

	static inline bool getRunNumber() {
		return runNumber_ ;
	}

	static void initialize(uint startBurstID, uint runNumber, std::function<void()> burstCleanupFunction) {
		currentBurstID_ = startBurstID;
		runNumber_ = runNumber;
		nextBurstId_ = currentBurstID_;
		running_ = true;
		flushBurst_ = false;
		burstCleanupFunction_ = burstCleanupFunction;
	}

	static void shutDown() {
		running_=false;
	}
	void thread();

	static void setEOBTime(uint eobTime) {
		eobTime_= eobTime;
    }
	static uint getEOBTime() {
		return eobTime_;
    }
	static void setSOBTime(uint sobTime) {
		sobTime_= sobTime;
    }
	static uint getSOBTime() {
		return sobTime_;
    }
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
	static uint runNumber_;
	static uint currentBurstID_;
	static std::atomic<bool> running_;
	static std::atomic<bool> flushBurst_;
	static std::function<void()> burstCleanupFunction_;
	static std::atomic<uint> eobTime_;
	static std::atomic<uint> sobTime_;
	static float flush_burst_s_;
	static float clean_burst_s_;
};

}
/* namespace na62 */

#endif /* BURSTIDHANDLER_H_ */
