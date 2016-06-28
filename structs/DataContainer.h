/*
 * DataContainer.h
 *
 *  Created on: Oct 23, 2015
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once

#include <netinet/in.h>
#include <sys/types.h>
#include <algorithm>
#include <cstdint>

#include "../options/Logging.h"

namespace na62 {
struct DataContainer {
	char * data;
	uint_fast16_t length;
	bool ownerMayFreeData;
	uint16_t checksum;
	in_port_t UDPPort;
	in_addr_t UDPAddr;


	DataContainer() :
			data(nullptr), length(0), ownerMayFreeData(false), checksum(0), UDPPort(0), UDPAddr(0) {
	}

	DataContainer(char* _data, uint_fast16_t _length, bool _ownerMayFreeData, in_port_t _UDPPort, in_addr_t _UDPAddr);

	~DataContainer() {
	}

	/**
	 * Copy constructor
	 */
	DataContainer(const DataContainer& other) :
			data(other.data), length(std::move(other.length)), ownerMayFreeData(
					other.ownerMayFreeData), checksum(other.checksum), UDPPort(other.UDPPort), UDPAddr(other.UDPAddr) {
	}

	/**
	 * Copy constructor
	 */
	DataContainer(const DataContainer&& other) :
			data(other.data), length(other.length), ownerMayFreeData(
					other.ownerMayFreeData), checksum(other.checksum), UDPPort(other.UDPPort),  UDPAddr(other.UDPAddr) {
	}

	/**
	 * Move assignment operator
	 */
	DataContainer& operator=(DataContainer&& other) {
		if (&other != this) {
			data = other.data;
			length = other.length;
			ownerMayFreeData = other.ownerMayFreeData;
			checksum = other.checksum;
			UDPPort =  other.UDPPort;
			UDPAddr = other.UDPAddr;

			other.data = nullptr;
			other.length = 0;
			other.UDPPort = 0;
		    other.UDPAddr = 0;

		}
		return *this;
	}

	/**
	 * Move assignment operator
	 */
	DataContainer& operator=(DataContainer& other) {
		if (&other != this) {
			data = other.data;
			length = other.length;
			ownerMayFreeData = other.ownerMayFreeData;
			checksum = other.checksum;
			UDPPort = other.UDPPort;
			UDPAddr = other.UDPAddr;
		}
		return *this;
	}

/*	bool checkValid();

	static inline u_int32_t Wrapsum(u_int32_t sum) {
		sum = ~sum & 0xFFFF;
		return (htons(sum));
	}

	static inline uint16_t GenerateChecksum(const char* data, int len,
			uint sum = 0) {
		return Wrapsum(GenerateChecksumUnwrapped(data, len, sum));
	}

	static inline uint16_t GenerateChecksumUnwrapped(const char* data, int len,
			uint64_t sum = 0) {
		int steps = len >> 2;
		while (steps > 0) {
			sum += ntohl(*((uint32_t *) data));
			data += sizeof(uint32_t);
			--steps;
		}

		if (len % sizeof(uint32_t) != 0) {
			uint remaining = len % sizeof(uint32_t);
			uint32_t add = 0;
			while (remaining-- > 0) {
				add += *(data++); // read next byte
				add = add << 8; // move all bytes to the left
			}
			sum += ntohl(add);
		}

		while (sum > 0xffffffffULL) {
			sum = (sum & 0xffffffffULL) + (sum >> 32);
		}
		sum = (sum & 0xffff) + (sum >> 16);
		sum += (sum >> 16);
		return sum;
	}*/

	void inline free() {
		//checkValid();
		if (ownerMayFreeData) {
			checksum = 0;
			delete[] data;
			data = nullptr;
			UDPPort = 0;
			UDPAddr = 0;

		}
	}
};

}
