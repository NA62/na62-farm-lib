/*
 * BurstFileWriter.h
 *
 *  Created on: Mar 20, 2015
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once

#include <stddef.h>
#include <sys/types.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <chrono>
#include <fstream>


#define WRITE_HDR

namespace na62 {
struct EVENT_HDR;
} /* namespace na62 */

namespace na62 {
struct BURST_HDR;
} /* namespace na62 */

namespace na62 {

class BurstFileWriter {

public:
	BurstFileWriter(const std::string filePath, const std::string fileName,
			const uint numberOfEvents, const uint sob, const uint runNumber,
			const uint burstID);

	~BurstFileWriter();

	void writeEvent(const EVENT_HDR* event);

	void writeBkmFile(const std::string bkmDir);

private:
	std::ofstream myFile_;
	std::string filePath_;
	std::string fileName_;
	BURST_HDR* hdr_;
	uint32_t* eventNumbers_;
	uint32_t* triggerWords_;
	uint32_t* offsets_;
	size_t bytesWritten_;
	uint eventID_;

	std::chrono::system_clock::time_point startTime_;
};

} /* namespace na62 */

