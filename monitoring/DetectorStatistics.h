/*
 * DetectorStatistics.h
 *
 *  Created on: May 17, 2016
 *      Author: NA62 collaboration
 */

#pragma once

#include <cstdlib>
#include <sys/types.h>
#include <atomic>
#include <sstream>
#include <string>

using namespace std;
namespace na62 {

class DetectorStatistics {
public:
	DetectorStatistics();
	virtual ~DetectorStatistics();

	static void init(int, int);
	static void shutdown();

	static void clearL0DetectorStatistics() {
		for (int i = 0; i < maxL0index; ++i) {
			for (int j = 0; j < 32; ++j) {
				L0receivedSourceIdsSubIds[i][j] = 0;
			}
		}
	}

	static void clearL1DetectorStatistics() {
		for (int i = 0; i < maxL1index; ++i) {
			for (int j = 0; j < 21; ++j) {
				L1receivedSourceIdsSubIds[i][j] = 0;
			}
		}
	}

	static void incrementL0stat(int detId, int detSubId) {
		L0receivedSourceIdsSubIds[detId / 4][detSubId].fetch_add(1,
				std::memory_order_relaxed);
	}

	static void incrementL1stat(int crate, int slot) {
		L1receivedSourceIdsSubIds[crate][slot].fetch_add(1,
				std::memory_order_relaxed);
	}

	static string L0RCInfo() {
		ostringstream s;
		for (int i = 0; i < maxL0index; ++i) {
			s << hex << i * 4 << "; " << dec;
			for (int j = 0; j < 32; ++j) {
				if (L0receivedSourceIdsSubIds[i][j] > 0)
					s << j << ":" << L0receivedSourceIdsSubIds[i][j] << " ";
			}
			s << "|";
		}
		return s.str();
	}
	static string L1RCInfo() {
		ostringstream s;
		for (int i = 0; i < maxL1index; ++i) {
			s << i << "; ";
			for (int j = 0; j < 21; ++j) {
				if (L1receivedSourceIdsSubIds[i][j] > 0)
					s << j << ":" << L1receivedSourceIdsSubIds[i][j] << " ";
			}
			s << "|";
		}
		return s.str();
	}

private:
	static int maxL0index;
	static int maxL1index;
	static std::atomic<uint>** L0receivedSourceIdsSubIds;
	static std::atomic<uint>** L1receivedSourceIdsSubIds;

};

} /* namespace na62 */

