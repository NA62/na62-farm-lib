/*
 * Utils.cpp
 *
 *  Created on: Apr 23, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "Utils.h"

#include "../options/Logging.h"
#include <iomanip>


namespace na62 {

std::string Utils::FormatSize(long int size) {
	int kB = 1000; // Kilobyte
	int MB = 1000 * kB; // Megabyte
	int GB = 1000 * MB; // Gigabyte

	std::string strNumber;

	char buffer[128];

	if (size < kB) {
		return std::to_string(size);
	}
	if (size < MB) {
		snprintf(buffer, sizeof(buffer), "%.2fk", (double) size / kB);
		return buffer;
	}
	if (size < GB) {
		snprintf(buffer, sizeof(buffer), "%.2fM", (double) size / MB);
		return buffer;
	}
	snprintf(buffer, sizeof(buffer), "%.2fG", (double) size / GB);
	return buffer;
}

double Utils::Average(std::vector<double> data) {
	double sum = 0;
	for (int i = data.size() - 1; i >= 0; i--) {
		sum += data[i];
	}
	return sum / data.size();
}

double Utils::StandardDevation(std::vector<double> data) {
	double av = Average(data);
	double sum = 0;

	for (int i = data.size() - 1; i >= 0; i--) {
		sum += (data[i] - av) * (data[i] - av);
	}

	return sqrt(sum / (data.size() - 1));
}

std::string Utils::PrintHex(const char* data, const size_t dataLength) {
	std::stringstream stream;
	for (uint_fast32_t i = 0; i < dataLength; i++) {
		uint_fast8_t byte;
		memcpy(&byte, &((char*) data)[i], 1);
		stream << std::hex << std::setw(2) << std::setfill('0') << (int) byte << " ";
		if ((i + 1) % 4 == 0) {
			stream << "\n";
		}
	}
	return stream.str();
}

uint64_t Utils::ToUInt(std::string str) throw (boost::bad_lexical_cast) {
	if (str.size() > 2 && str.substr(0, 2) == "0x") {
		uint x;
		std::stringstream ss;
		ss << std::hex << str.substr(2, str.size() - 2);
		ss >> x;
		return x;
	} else {
		return boost::lexical_cast<uint64_t>(str);
	}
}


} /* namespace na62 */
