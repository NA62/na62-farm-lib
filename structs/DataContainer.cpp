/*
 * DataContainer.h
 *
 *  Created on: Oct 23, 2015
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "DataContainer.h"

namespace na62 {

DataContainer::DataContainer(char* _data, uint_fast16_t _length) :
		data(_data), length(_length) {
	checksum = GenerateChecksum(_data, _length, 0);
}

bool DataContainer::checkValid() {
	if(checksum==0 && data == nullptr){
		return true;
	}

	if (checksum != GenerateChecksum(data, length, 0)) {
		LOG_ERROR<<"Packet broke!" << ENDL;
	}
	return checksum != GenerateChecksum(data, length, 0);
}

}
