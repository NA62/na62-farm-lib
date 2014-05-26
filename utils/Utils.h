/*
 * Utils.h
 *
 *  Created on: Apr 23, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include <stdint.h>
#include <boost/lexical_cast.hpp>
#include <vector>

#ifndef UTILS_H_
#define UTILS_H_

namespace na62 {

class Utils {
public:
	static std::string FormatSize(long int size);
	static double Average(std::vector<double> data);
	static double StandardDevation(std::vector<double> data);
	static void PrintHex(const char* data, const size_t dataLength);
	static uint ToUInt(std::string str) throw (boost::bad_lexical_cast);

	static inline uint8_t highbit(uint8_t& t) {
		return t = (((uint8_t) (-1)) >> 1) + 1;
	}

	static std::ostream& bin(uint8_t& value, std::ostream &o) {
		for (uint8_t bit = highbit(bit); bit; bit >>= 1) {
			o << ((value & bit) ? '1' : '0');
		}
		return o;
	}

};
} /* namespace na62 */
#endif /* UTILS_H_ */
