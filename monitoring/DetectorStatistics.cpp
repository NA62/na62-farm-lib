/*
 * DetectorStatistics.cpp
 *
 *  Created on: May 17, 2016
 *      Author: NA62 collaboration - R. Fantechi
 */

#include "DetectorStatistics.h"

#include <sys/types.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <ctime>
#include <chrono>
#include <atomic>

namespace na62 {
int DetectorStatistics::maxL0index;
int DetectorStatistics::maxL1index;
std::atomic<uint>** DetectorStatistics::L0receivedSourceIdsSubIds;
std::atomic<uint>** DetectorStatistics::L1receivedSourceIdsSubIds;

DetectorStatistics::DetectorStatistics() {

}

DetectorStatistics::~DetectorStatistics() {
}

void DetectorStatistics::init(int maxL0, int maxL1) {
	L0receivedSourceIdsSubIds = new std::atomic<uint>*[maxL0];
	for (int i = 0; i < maxL0; i++)
		L0receivedSourceIdsSubIds[i] = new std::atomic<uint>[32];
	L1receivedSourceIdsSubIds = new std::atomic<uint>*[maxL1];
	for (int i = 0; i < maxL1; i++)
		L1receivedSourceIdsSubIds[i] = new std::atomic<uint>[21];
	maxL0index = maxL0;
	maxL1index = maxL1;
}

void DetectorStatistics::shutdown() {
}

} /* namespace na62 */
