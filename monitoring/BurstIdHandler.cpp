/*
 * BurstIdHandler.cpp
 *
 *  Created on: May 27, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "BurstIdHandler.h"

#include <sys/types.h>
#include <sstream>
#include <string>
#include <utility>
#include <tbb/tbb.h>

#include "../options/Logging.h"
#include "../eventBuilding/Event.h"
#include "../eventBuilding/EventPool.h"
#include "../eventBuilding/SourceIDManager.h"
#include "../utils/DataDumper.h"
#include "../l0/MEPFragment.h"
#include "../l0/Subevent.h"
#include "../structs/L0TPHeader.h"

namespace na62 {
boost::timer::cpu_timer BurstIdHandler::EOBReceivedTimer_;
std::mutex BurstIdHandler::timerMutex_;
uint BurstIdHandler::nextBurstId_;
uint BurstIdHandler::currentBurstID_;

std::atomic<bool> BurstIdHandler::running_(false);
std::atomic<bool> BurstIdHandler::flushBurst_(false);
std::atomic<uint> BurstIdHandler::incompleteEvents_(0);

void BurstIdHandler::thread(){
	while(BurstIdHandler::running_) {
		//LOG_INFO<< "Burst ID Handler thread " << (bool) BurstIdHandler::isInBurst() << " " << (bool)BurstIdHandler::flushBurst() <<  " " << (int) BurstIdHandler::getTimeSinceLastEOB();
		if (BurstIdHandler::isInBurst() == false && BurstIdHandler::flushBurst_ == false && BurstIdHandler::getTimeSinceLastEOB() > 3.) {
			// Mark that all further data shall be discarded
			LOG_INFO<< "Preparing end of burst " << (int) BurstIdHandler::getCurrentBurstId();
			BurstIdHandler::flushBurst_=true;
		}
		else if (  BurstIdHandler::isInBurst() == false && BurstIdHandler::flushBurst_ == true && BurstIdHandler::getTimeSinceLastEOB() > 5.) {
			// Flush all events
			LOG_INFO<< "Cleanup of burst " << (int) BurstIdHandler::getCurrentBurstId();
			onBurstFinished();
			BurstIdHandler::currentBurstID_ = BurstIdHandler::nextBurstId_;
			BurstIdHandler::flushBurst_ = false;

			LOG_INFO<< "Start of burst " << (int) BurstIdHandler::getCurrentBurstId();

		}
		// Slow down polling
		boost::this_thread::sleep(boost::posix_time::microsec(100000));
	}
}

void BurstIdHandler::onBurstFinished() {
	int maxNumOfPrintouts = 100;
    incompleteEvents_ = 0;

#ifdef HAVE_TCMALLOC
    // Do it with parallel_for using tbb if tcmalloc is linked
	tbb::parallel_for(
			tbb::blocked_range<uint_fast32_t>(0, EventPool::getLargestTouchedEventnumberIndex() + 1,
					EventPool::getLargestTouchedEventnumberIndex() / std::thread::hardware_concurrency()),
					[](const tbb::blocked_range<uint_fast32_t>& r) {
		for(size_t index=r.begin();index!=r.end(); index++) {
			Event* event = EventPool::getEventByIndex(index);
			if(event == nullptr) continue;
			if (event->isUnfinished()) {
				++incompleteEvents_;
				event->updateMissingEventsStats();
				EventPool::freeEvent(event);
			}
		}
	});
#else
	for (uint idx = 0; idx != EventPool::getLargestTouchedEventnumberIndex() + 1; ++idx) {
		Event* event = EventPool::getEventByIndex(idx);
		if (event->isUnfinished()) {
			++incompleteEvents_;
			// if EOB send event to merger as in L2Builder.cpp
			EventPool::freeEvent(event);
		}
	}
#endif

	if(incompleteEvents_ > 0) {
		LOG_ERROR << "Dropped " << incompleteEvents_ << " events in burst ID = " << (int) BurstIdHandler::getCurrentBurstId() << ".";
	}
}

} /* namespace na62 */
