/*
 * BurstIdHandler.cpp
 *
 *  Created on: May 27, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "BurstIdHandler.h"

//#include "../eventBuilding/Event.h"
//#include "../eventBuilding/EventPool.h"
//#include "../eventBuilding/SourceIDManager.h"
//#include "../utils/DataDumper.h"
//#include "../l0/MEPFragment.h"
//#include "../l0/Subevent.h"
//#include "../structs/L0TPHeader.h"

namespace na62 {
boost::timer::cpu_timer BurstIdHandler::EOBReceivedTimer_;
std::mutex BurstIdHandler::timerMutex_;
uint BurstIdHandler::nextBurstId_;
uint BurstIdHandler::runNumber_ = 0;
uint BurstIdHandler::currentBurstID_ = 0;
std::atomic<uint> BurstIdHandler::eobTime_(0);
std::atomic<uint> BurstIdHandler::sobTime_(0);

std::atomic<bool> BurstIdHandler::running_(false);
std::atomic<bool> BurstIdHandler::flushBurst_(false);
std::function<void()> BurstIdHandler::burstCleanupFunction_(nullptr);

float BurstIdHandler::flush_burst_s_(3.);
float BurstIdHandler::clean_burst_s_(5.);


void BurstIdHandler::thread() {
	while(BurstIdHandler::running_) {
		//LOG_INFO("Burst ID Handler thread " << (bool) BurstIdHandler::isInBurst() << " " << (bool)BurstIdHandler::flushBurst() <<  " " << (int) BurstIdHandler::getTimeSinceLastEOB());
		if (BurstIdHandler::isInBurst() == false and BurstIdHandler::flushBurst_ == false and BurstIdHandler::getTimeSinceLastEOB() > flush_burst_s_) {
			// Mark that all further data shall be discarded
			LOG_INFO("Preparing end of burst " << (int) BurstIdHandler::getCurrentBurstId());
			BurstIdHandler::flushBurst_ = true;
		} else if (BurstIdHandler::isInBurst() == false and BurstIdHandler::flushBurst_ == true and BurstIdHandler::getTimeSinceLastEOB() > clean_burst_s_) {
			// Flush all events
			LOG_INFO("Cleanup of burst " << (int) BurstIdHandler::getCurrentBurstId());
			//onBurstFinished();
			BurstIdHandler::burstCleanupFunction_();
			BurstIdHandler::currentBurstID_ = BurstIdHandler::nextBurstId_;
			BurstIdHandler::flushBurst_ = false;

			LOG_INFO("Start of burst " << (int) BurstIdHandler::getCurrentBurstId());

		}
		// Slow down polling
		boost::this_thread::sleep(boost::posix_time::microsec(100000));
	}
}
} /* namespace na62 */
