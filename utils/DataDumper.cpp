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
#ifdef USE_GLOG
#include <glog/logging.h>
#endif

namespace na62 {

std::string generateFreeFilePath(std::string fileName,
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

	std::cout << "Writing file " << filePath << std::endl;

	if (!generateDirIfNotExists(storageDir)) {
		return;
	}

	std::ofstream myfile;
	myfile.open(filePath.data(),
			std::ios::out | std::ios::trunc | std::ios::binary);

	if (!myfile.good()) {
		std::cerr << "Unable to write to file " << filePath << std::endl;
		// carry on to free the memory. myfile.write will not throw!
	} else {
		myfile.write(data, length);
	}

	myfile.close();
}

bool generateDirIfNotExists(const std::string dirPath) {
	if (!boost::filesystem::exists(dirPath)) {
		if (!boost::filesystem::create_directory(dirPath)) {
			std::cerr << "Unable to write to file " << dirPath << std::endl;
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
			std::cerr << "Unable to write to file " << filePath << std::endl;
			return;
		}
	}

	std::ofstream myfile;
	myfile.open(filePath.data(), std::ios::out | std::ios::app);

	if (!myfile.good()) {
		std::cerr << "Unable to write to file " << filePath << std::endl;
		// carry on to free the memory. myfile.write will not throw!
	} else {
		myfile.write(message.c_str(), message.length());
		myfile.write("\n", 1);
	}

	myfile.close();
}

}
