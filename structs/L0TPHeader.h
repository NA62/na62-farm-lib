/*
 * L0TPHeader.h
 *
 *  Created on: Oct 10, 2014
 *      Author: Jonas Kunze <kunze.jonas@gmail.com>
 */

#ifndef L0TPHEADER_H_
#define L0TPHEADER_H_

#include <stdint.h>

struct L0TpHeader { // 24 bytes
	uint_fast8_t refFineTime;
	uint_fast8_t dataType;
	uint_fast16_t primitives[7];

	// 3 words primitives

	uint_fast16_t previousTimeStampHigh;
	uint_fast16_t memoryAddress;

	uint_fast8_t l0TriggerType;
	uint_fast8_t previousl0TriggerType;
	uint_fast16_t previousTimeStampLow;

}__attribute__ ((__packed__));

#endif /* L0TPHEADER_H_ */
