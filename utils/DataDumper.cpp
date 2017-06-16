/*
 * DataDumper.cpp
 *
 *  Created on: May 7, 2014
 \*      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "DataDumper.h"

#include<boost/filesystem.hpp>
#include <fstream>
#include <stddef.h>
#include <sys/types.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include "../options/Logging.h"

namespace na62 {

std::string DataDumper::generateFreeFilePath(std::string fileName,
		const std::string storageDir) {
	std::string filePath = storageDir + "/" + fileName;
	if (boost::filesystem::exists(filePath)) {
		int counter = 2;
		std::string tmpName = fileName;
		do {
			tmpName = fileName + "_" + std::to_string(++counter);
		} while (boost::filesystem::exists(storageDir + "/" + tmpName));

		fileName = tmpName;
		filePath = storageDir + "/" + fileName;
	}

	return filePath;
}

void DataDumper::dumpToFile(std::string fileName, const std::string storageDir,
		const char* data, const uint length) {

	const std::string filePath = generateFreeFilePath(fileName, storageDir);

	LOG_INFO("Writing file " << filePath);

	if (!generateDirIfNotExists(storageDir)) {
		return;
	}

	std::ofstream myfile;
	myfile.open(filePath.data(),
			std::ios::out | std::ios::trunc | std::ios::binary);

	if (!myfile.good()) {
		LOG_ERROR("Unable to write to file " << filePath);
		// carry on to free the memory. myfile.write will not throw!
	} else {
		myfile.write(data, length);
	}
	LOG_INFO("Closing file " << filePath);
	myfile.close();
}

bool DataDumper::generateDirIfNotExists(const std::string dirPath) {
	if (!boost::filesystem::exists(dirPath)) {
		if (!boost::filesystem::create_directory(dirPath)) {
			LOG_ERROR("Unable to write to file " << dirPath);
			return false;
		}
	}
	return true;
}

void DataDumper::printToFile(std::string fileName, const std::string storageDir,
		const std::string message) {
	std::string filePath = storageDir + "/" + fileName;

	if (!boost::filesystem::exists(storageDir)) {
		if (!boost::filesystem::create_directory(storageDir)) {
			LOG_ERROR("Unable to write to file " << filePath);
			return;
		}
	}

	std::ofstream myfile;
	myfile.open(filePath.data(), std::ios::out | std::ios::app);

	if (!myfile.good()) {
		LOG_ERROR("Unable to write to file " << filePath);
		// carry on to free the memory. myfile.write will not throw!
	} else {
		myfile.write(message.c_str(), message.length());
		myfile.write("\n", 1);
	}

	myfile.close();
}
}
