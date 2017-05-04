/*
 * L0TPHeader.h
 *
 *  Created on: Oct 10, 2014
 *      Author: Jonas Kunze <kunze.jonas@gmail.com>
 */

#ifndef L0TPHEADER_H_
#define L0TPHEADER_H_

#include <stdint.h>

struct L0TpHeader { // 32 bytes
	uint8_t refFineTime;
	uint8_t dataType;
	uint16_t primitives[7];

	// 3 words primitives

	uint32_t previousTimeStamp;
	uint8_t l0TriggerType;
	uint8_t previousl0TriggerType;
	uint16_t l0TriggerFlags;

	uint8_t primFineTime[7];
	uint8_t l0dummy;

}__attribute__ ((__packed__));

#endif /* L0TPHEADER_H_ */
