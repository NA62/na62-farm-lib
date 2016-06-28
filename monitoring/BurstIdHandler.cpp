/*
 * BurstIdHandler.cpp
 *
 *  Created on: May 27, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "BurstIdHandler.h"

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sstream>
#include <string>
#include <utility>
//#include <socket/EthernetUtils.h>
//#include <monitoring/IPCHandler.h>
#include "../options/Logging.h"
#include "../eventBuilding/Event.h"
#include "../eventBuilding/EventPool.h"
#include "../eventBuilding/SourceIDManager.h"
#include "../utils/DataDumper.h"
#include "../l0/MEPFragment.h"
#include "../l0/Subevent.h"
#include "../structs/L0TPHeader.h"
#include <boost/asio.hpp>

namespace na62 {
boost::timer::cpu_timer BurstIdHandler::EOBReceivedTimer_;
std::mutex BurstIdHandler::timerMutex_;
uint BurstIdHandler::nextBurstId_;
uint BurstIdHandler::currentBurstID_;
#ifdef USE_SIMU
uint BurstIdHandler::auto_inc_;
uint BurstIdHandler::secs_;
std::string BurstIdHandler::deviceName_;
std::vector<std::pair<int, int> > BurstIdHandler::sourceIDs_;
#endif
std::atomic<bool> BurstIdHandler::running_(false);
std::atomic<bool> BurstIdHandler::flushBurst_(false);
std::function<void()> BurstIdHandler::burstCleanupFunction_(nullptr);

void BurstIdHandler::thread(){

	while(BurstIdHandler::running_) {
#ifdef USE_SIMU
		if (BurstIdHandler::autoInc() == 0){
#endif


			//LOG_INFO("Burst ID Handler thread " << (bool) BurstIdHandler::isInBurst() << " " << (bool)BurstIdHandler::flushBurst() <<  " " << (int) BurstIdHandler::getTimeSinceLastEOB());
			if (BurstIdHandler::isInBurst() == false && BurstIdHandler::flushBurst_ == false && BurstIdHandler::getTimeSinceLastEOB() > 3.) {
				// Mark that all further data shall be discarded
				LOG_INFO("Preparing end of burst " << (int) BurstIdHandler::getCurrentBurstId());
				BurstIdHandler::flushBurst_=true;
			}
			else if (  BurstIdHandler::isInBurst() == false && BurstIdHandler::flushBurst_ == true && BurstIdHandler::getTimeSinceLastEOB() > 5.) {
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

			//IPCHandler::sendCommand("eob_timestamp:1");
#ifdef USE_SIMU
		}else{
			time_t start = time(0);
			time_t timeLeft = (time_t) BurstIdHandler::secondsBID();

			while ((timeLeft > 0))
			{
				time_t end = time(0);
				time_t timeTaken = end - start;
				timeLeft = BurstIdHandler::secondsBID() - timeTaken;

			}
			//std::string myIP = EthernetUtils::ipToString(EthernetUtils::GetIPOfInterface(deviceName_));
			boost::asio::io_service io_service;
			boost::asio::ip::udp::resolver resolver(io_service);
	        boost::asio::ip::udp::endpoint receiver_endpoint;
			receiver_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("10.194.20.9"), 55555);
	        boost::asio::ip::udp::socket socket(io_service);
     		socket.open(boost::asio::ip::udp::v4());
       	   	socket.send_to(boost::asio::buffer("stop", sizeof("stop")), receiver_endpoint);

			LOG_INFO("Preparing end of burst ***************************************************" << (int) BurstIdHandler::getCurrentBurstId());
			BurstIdHandler::flushBurst_= true;
			LOG_INFO("Cleanup of burst ********************************************************** " << (int) BurstIdHandler::getCurrentBurstId());
			BurstIdHandler::burstCleanupFunction_();
			BurstIdHandler::setNextBurstID(BurstIdHandler::currentBurstID_ + 1);
			BurstIdHandler::currentBurstID_ = BurstIdHandler::nextBurstId_;
			BurstIdHandler::flushBurst_ = false;
			FarmStatistics::init();
			LOG_INFO("Start of burst " << (int) BurstIdHandler::getCurrentBurstId());


			// Slow down polling
			//boost::this_thread::sleep(boost::posix_time::microsec(100000));


		} //end else
#endif

	} //end while running

}
} /* namespace na62 */
