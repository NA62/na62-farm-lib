/*
 * DataDumper.h
 *
 *  Created on: May 7, 2014
 \*      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef DATADUMPER_H_
#define DATADUMPER_H_

#include <sys/types.h>
#include <string>

namespace na62 {
class DataDumper {
public:
	/**
	 * Dumps the given data into the file [fileName] in the directory [storageDir]. If [fileName]
	 * already exists _XXX will be concatenated to [fileName]
	 */
	static void dumpToFile(std::string fileName, const std::string storageDir,
			const char* data, const uint length);

	static void printToFile(std::string fileName, const std::string storageDir,
			const std::string message);

	/**
	 * Concatenates fileName and storageDir and checks if this file already exists. If it does, it will append _X, with X being
	 * the first free integer
	 */
	static std::string generateFreeFilePath(std::string fileName, const std::string storageDir);

	/**
	 * Creates the directory [dirPath] and returns true in case of success or if it already existed and false in case of an error
	 */
	static bool generateDirIfNotExists(const std::string dirPath);

};
}

#endif /* DATADUMPER_H_ */
