/*
 * FarmStatistics.cpp
 *
 *  Created on: 05.11.2015
 *      Author: Tassilo
 */

#include <tbb/task.h>
#include <tbb/tick_count.h>
#include <tbb/tbb_thread.h>
#include <sys/types.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <stdio.h>
#include <time.h>
#include "../utils/AExecutable.h"
#include <unistd.h>
#include <ctime>
#include <chrono>

#include <boost/timer/timer.hpp>
#include "FarmStatistics.h"
namespace na62 {
std::atomic<uint> FarmStatistics::PH;
std::atomic<uint> FarmStatistics::T;
std::atomic<uint> FarmStatistics::LB;
std::atomic<uint> FarmStatistics::LP;
std::vector<statisticTimeStamp> FarmStatistics::recvTimes;
std::vector<statisticTimeStamp> FarmStatistics::recvTimesBuff;
boost::timer::cpu_timer FarmStatistics::timer;
std::chrono::steady_clock::time_point FarmStatistics::t1;
std::chrono::steady_clock::time_point FarmStatistics::t2;
bool FarmStatistics::running_;
char* FarmStatistics::hostname;

FarmStatistics::FarmStatistics() {
	startRunning();
}

FarmStatistics::~FarmStatistics() {
	// TODO Auto-generated destructor stub
}

void FarmStatistics::init() {
//	FarmStatistics::timer.start();LOG_INFO("started timer" << std::to_string(timer.elapsed().wall));
	FarmStatistics::t1 = std::chrono::steady_clock::now();
	FarmStatistics::hostname = getHostName();LOG_INFO("got Hostname: " << std::string(hostname));
	FarmStatistics::recvTimes.reserve(200);LOG_INFO("reserved memory");
	FarmStatistics::recvTimesBuff.reserve(200);LOG_INFO("reserved buffer");
}

void FarmStatistics::thread() {
	std::ofstream myfile;
	std::string filename = "/performance/log/" + currentDateTime() + "_" + "localhost"/*std::string(hostname)*/ + ".txt";
	const char* filenamechars = filename.c_str();
	myfile.open(filenamechars, std::ofstream::app);
	while (running_) {
		//TODO add CAS
		if (FarmStatistics::recvTimes.size() >= 1) {
			FarmStatistics::recvTimesBuff = FarmStatistics::recvTimes;
			FarmStatistics::recvTimes.clear();
			for (statisticTimeStamp a : recvTimesBuff) {
				myfile << getFileOutString(a);
			}
		}
		boost::this_thread::sleep(boost::posix_time::microsec(1000));
	}
	myfile.close();
}
//I just removed static and i add FarmStatistics in front
 void FarmStatistics::addTime(std::string comment) {
	statisticTimeStamp st;
	FarmStatistics::t2 = std::chrono::steady_clock::now();
	std::chrono::steady_clock::duration time_span = FarmStatistics::t2 - FarmStatistics::t1;
	st.time = u_int64_t(time_span.count());
	st.comment = comment;
	FarmStatistics::recvTimes.push_back(st);
}

 uint FarmStatistics::getID(int source) {
	uint idNo=0;
	switch (source) {
	case 1:
		FarmStatistics::PH.fetch_add(1, std::memory_order_relaxed);
		idNo = unsigned(PH);
		break;
	case 2:
		FarmStatistics::T.fetch_add(1, std::memory_order_relaxed);
		idNo = unsigned(T);
		break;
	case 3:
		FarmStatistics::LB.fetch_add(1, std::memory_order_relaxed);
		idNo = unsigned(LB);
		break;
	case 4:
		FarmStatistics::LP.fetch_add(1, std::memory_order_relaxed);
		idNo = unsigned(LP);
		break;
	}
	return idNo;
}

// gethostname seems to crash
 char* FarmStatistics::getHostName() {
//	20 chars should fit all hostnames
	 char* host = new char[32];
	 gethostname(host,32);
	 return host;
	//if (!(hostname = std::getenv("HOSTNAME"))) {
	//	LOG_INFO("error at hostname retrieval");
	//	return nullptr;
	//}
	//return hostname;
}

// Buid the line to be written into the logfile
std::string FarmStatistics::getFileOutString(statisticTimeStamp sts) {
	std::string fos(
			sts.comment + ", \ttime: " + std::to_string(double(sts.time)/1000000) + "(" + std::to_string(sts.time) + ")" + "\n");
	return fos;
}

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
std::string FarmStatistics:: currentDateTime() {
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

	return buf;
}
}
