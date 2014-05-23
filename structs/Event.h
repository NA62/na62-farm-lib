/*
 * Event.hpp
 *
 *  Created on: Mar 7, 2014
 \*      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef EVENT_HPP_
#define EVENT_HPP_

#include <cstdint>

#include "Network.h"

namespace na62 {
/*
 * UDP/IP complete header
 *
 */
struct EVENT_HDR {
	uint32_t eventNum :24;
	uint8_t format;

	uint32_t length;
	uint32_t burstID;
	uint32_t timestamp;

	uint32_t triggerWord :24;
	uint8_t reserved1;

	uint8_t fineTime;
	uint8_t numberOfDetectors;
	uint16_t reserved2;

	uint32_t processingID;

	uint32_t SOBtimestamp;
}__attribute__ ((__packed__));

/*
 * Header for each L0 data block coming from one Tel62 board. This allows to recognize the data block end even if the data coming
 * from the electronics is broken
 *
 */
struct L0_BLOCK_HDR {
	uint16_t dataBlockSize;
	uint8_t sourceSubID;
	uint8_t reserved;
}__attribute__ ((__packed__));

struct EVENT_TRAILER {
	uint32_t eventNum :24;
	uint8_t reserved;
}__attribute__ ((__packed__));

struct EOB_FULL_FRAME {
	struct UDP_HDR udp;
	uint32_t finishedBurstID;
	uint32_t lastEventNum :24;
	uint8_t reserved;
}__attribute__ ((__packed__));

}
#endif /* EVENT_HPP_ */
