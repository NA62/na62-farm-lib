/*
 * DataContainer.h
 *
 *  Created on: Oct 23, 2015
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "DataContainer.h"

namespace na62 {
uint16_t DataContainer::BufferInUseWord = 0;
DataContainer::DataContainer(char* _data, uint_fast16_t _length,
		bool isPfRingBuffer) :
		data(_data), length(_length), isPfRingBuffer(isPfRingBuffer) {
	checksum = GenerateChecksum(_data, _length, 0);

	if (isPfRingBuffer) {
		if (getBufferInUse(_data)) {
			LOG_ERROR<< "Overwriting a used buffer: " << (long long) _data
			<< ENDL;
		}
		setBufferInUse(_data, true);
	}
}

bool DataContainer::checkValid() {
	if (checksum == 0 && data == nullptr) {
		return true;
	}

	if (checksum != GenerateChecksum(data, length, 0)) {
		LOG_ERROR<<"Packet broke!" << ENDL;
	}
	return checksum != GenerateChecksum(data, length, 0);
}

}
