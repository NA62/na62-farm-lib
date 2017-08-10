/*
 * BurstFileWriter.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "BurstFileWriter.h"

#include <boost/date_time/microsec_time_clock.hpp>
#include <boost/date_time/posix_time/posix_time_config.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/time.hpp>
#include <boost/date_time/time_duration.hpp>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>


#include "../options/Logging.h"
#include "../structs/BurstFile.h"
#include "../structs/Event.h"
#include "../utils/Utils.h"

namespace na62 {

BurstFileWriter::BurstFileWriter(const std::string filePath,
		const std::string fileName, const uint numberOfEvents, const uint sob,
		const uint runNumber, const uint burstID) :
		myFile_(filePath.data(),
				std::ios::out | std::ios::trunc | std::ios::binary), filePath_(
				filePath), fileName_(fileName), eventID_(0) {

	if (!myFile_.good()) {
		LOG_ERROR("Unable to write to file " << filePath);
		exit(1);
	}

	/*
	 * Generate the burst file header
	 */
	const uint headerLength = BURST_HDR::calculateHeaderSize(numberOfEvents);
	hdr_ = reinterpret_cast<BURST_HDR*>(new char[headerLength]);

	hdr_->fileFormatVersion = 1;
	hdr_->zero = 0;

	hdr_->numberOfEvents = numberOfEvents;
	hdr_->runID = runNumber;
	hdr_->burstID = burstID;

	eventNumbers_ = hdr_->getEventNumbers();
	triggerWords_ = hdr_->getEventTriggerTypeWords();
	offsets_ = hdr_->getEventOffsets();

	bytesWritten_ = headerLength;

	stopWatch_.start();
#ifdef WRITE_HDR
	// jump to the first byte behind the header
	myFile_.seekp(headerLength);
#endif
}

BurstFileWriter::~BurstFileWriter() {
#ifdef WRITE_HDR
	// Write the header to the beginning
	myFile_.seekp(0);
	myFile_.write(reinterpret_cast<const char*>(hdr_), hdr_->getHeaderSize());
	myFile_.close();
#endif
	boost::posix_time::ptime stop(
			boost::posix_time::microsec_clock::local_time());

	long msec = stopWatch_.elapsed().wall / 1E6;
	long dataRate = 0;
	if (msec != 0) {
		dataRate = bytesWritten_ / msec * 1000; // B/s
	}

	LOG_INFO("Wrote burst " << hdr_->burstID << " with " << hdr_->numberOfEvents << " events and " << bytesWritten_ << "B with " << Utils::FormatSize(dataRate) << "B/s");

	delete[] hdr_;
}

bool BurstFileWriter::doChown(std::string file_path, std::string user_name, std::string group_name) {

	struct passwd* pwd = getpwnam(user_name.c_str());
	if (pwd == NULL) {
		LOG_ERROR("Failed to get uid");
		raise;
	}
	uid_t uid = pwd->pw_uid;

	struct group* grp = getgrnam(group_name.c_str());
	if (grp == NULL) {
		LOG_ERROR("Failed to get gid");
		raise;
	}
	gid_t  gid = grp->gr_gid;

	if (chown(file_path.c_str(), uid, gid) == -1) {
		LOG_ERROR("chown fail");
		return false;
	}
	return true;
}

void BurstFileWriter::writeEvent(const EVENT_HDR* event) {
	myFile_.write(reinterpret_cast<const char*>(event), event->length * 4);

	eventNumbers_[eventID_] = event->eventNum;
	triggerWords_[eventID_] = event->triggerWord;
	offsets_[eventID_] = bytesWritten_ / 4;
	bytesWritten_ += event->length * 4;
	eventID_++;
}

void BurstFileWriter::writeBkmFile(const std::string bkmDir) {
	time_t rawtime;
	struct tm * timeinfo;
	char timeString[24];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(timeString, 64, "%d-%m-%y_%H:%M:%S", timeinfo);

	std::string BKMFilePath = bkmDir + "/" + fileName_;

	std::ofstream BKMFile;
	BKMFile.open(BKMFilePath.data(), std::ios::out | std::ios::trunc);

	if (!BKMFile.good()) {
		LOG_ERROR("Unable to write to file " << BKMFilePath << "");
		return;
	}

	BKMFile.write(filePath_.data(), filePath_.length());
	BKMFile.write("\n", 1);

	std::string sizeLine = "size: " + std::to_string(bytesWritten_);
	BKMFile.write(sizeLine.data(), sizeLine.length());
	BKMFile.write("\n", 1);

	std::string dateLine = "datetime: " + std::string(timeString);
	BKMFile.write(dateLine.data(), dateLine.length());
	BKMFile.write("\n", 1);

	BKMFile.close();

	system(std::string("chown na62cdr:vl " + BKMFilePath).data());

	LOG_INFO("Wrote BKM file " << BKMFilePath);
}

}
/* namespace na62 */
