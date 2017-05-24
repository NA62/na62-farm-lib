/*
 * Event.hpp
 *
 *  Created on: Mar 7, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once

#include <cstdint>

namespace na62 {

struct EobDataHdr {
	u_int16_t length; // number of 32-bit words including this header
	u_int8_t blockID;
	u_int8_t detectorID;

	u_int32_t eobTimestamp;
}__attribute__ ((__packed__));

typedef struct l1EobMaskCounter_t {
	uint32_t L1ReservedPerMask;
	uint32_t L1InputEventsPerMask;
	uint32_t L1AcceptedEventsPerMask;
} l1EobMaskCounter;

typedef struct l2EobMaskCounter_t {
	uint32_t L2ReservedPerMask;
	uint32_t L2InputEventsPerMask;
	uint32_t L2AcceptedEventsPerMask;
} l2EobMaskCounter;

typedef struct l1EobCounter_t {
	uint8_t formatVersion;
	uint8_t timeoutFlag;
	uint16_t reserved;
	uint32_t extraReserved;
	uint32_t L1InputEvents;
	uint32_t L1SpecialEvents;
	uint32_t L1ControlEvents;
	uint32_t L1PeriodicsEvents;
	uint32_t L1PhysicsEvents;
	uint32_t L1PhysicsEventsByMultipleMasks;
	uint32_t L1RequestToCreams;
	uint32_t L1OutputEvents;
	uint32_t L1AcceptedEvents;
	uint32_t L1TimeoutEvents;
	uint32_t L1AllDisabledEvents;
	uint32_t L1BypassEvents;
	uint32_t L1FlagAlgoEvents;
	uint32_t L1AutoPassEvents;
	l1EobMaskCounter l1Mask[16]; //for 16 L0 masks
} l1EobCounter;

typedef struct l2EobCounter_t {
	uint8_t formatVersion;
	uint8_t timeoutFlag;
	uint16_t reserved;
	uint32_t extraReserved;
	uint32_t L2InputEvents;
	uint32_t L2SpecialEvents;
	uint32_t L2ControlEvents;
	uint32_t L2PeriodicsEvents;
	uint32_t L2PhysicsEvents;
	uint32_t L2PhysicsEventsByMultipleMasks;
	uint32_t L2OutputEvents;
	uint32_t L2AcceptedEvents;
	uint32_t L2TimeoutEvents;
	uint32_t L2AllDisabledEvents;
	uint32_t L2BypassEvents;
	uint32_t L2FlagAlgoEvents;
	uint32_t L2AutoPassEvents;
	l2EobMaskCounter l2Mask[16]; //for 16 L0 masks
} l2EobCounter;

typedef struct l1EOBInfo_t { //add here burstID
	EobDataHdr header;
	l1EobCounter l1EobData;
} l1EOBInfo;

typedef struct l2EOBInfo_t { //add here burstID
	EobDataHdr header;
	l2EobCounter l2EobData;
} l2EOBInfo;

}
