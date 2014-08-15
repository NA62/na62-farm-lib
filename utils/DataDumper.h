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
	static void dumpToFile(std::string fileName,
			const std::string storageDir, const char* data, const uint length);

	static void printToFile(std::string fileName,
			const std::string storageDir, const std::string message);
};
}

#endif /* DATADUMPER_H_ */
