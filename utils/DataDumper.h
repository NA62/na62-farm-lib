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
	static void dumpToFile(const std::string fileName,
			const std::string storageDir, const char* data, const uint length);
};
}

#endif /* DATADUMPER_H_ */
