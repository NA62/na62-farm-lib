/*
 * L1DistributionHandler.h
 *
 *  Created on: Mar 3, 2012
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#pragma once
#ifndef L1DISTRIBUTIONHANDLER_H_
#define L1DISTRIBUTIONHANDLER_H_

#include <atomic>
#include <cstdbool>
#include <cstdint>
#include <utility>
#include <vector>

#include "../options/Options.h"
#include "../utils/ThreadsafeQueue.h"
#include "MRP.h"

namespace na62 {
class Event;
namespace cream {

typedef std::pair<struct TRIGGER_RAW_HDR*, std::vector<uint16_t> > unicastTriggerAndCrateCREAMIDs_type;

class L1DistributionHandler {
public:
	static void StartThread();

	static void Async_RequestLKRDataMulticast(const uint16_t& threadNum, Event *event, bool zSuppressed);
	static void Async_RequestLKRDataUnicast(const uint16_t& threadNum, const Event *event, bool zSuppressed,
			const std::vector<uint16_t> crateCREAMIDs);

	static inline uint64_t GetL1TriggersSent() {
		return L1DistributionHandler::L1TriggersSent;
	}

	static inline uint64_t GetL1MRPsSent() {
		return L1DistributionHandler::L1MRPsSent;
	}

	/*
	 * Allows an EB to stop the MRP transmission
	 */
	static inline void PauseSending() {
		paused = true;
	}

	/*
	 * Allows an EB to restart the MRP transmission
	 */
	static inline void ResumeSending() {
		paused = false;
	}

	static inline bool SendingIsPaused() {
		return paused;
	}

private:
	/*
	 * Will send all the Triggers in <triggers> with the given <dataHDR>
	 * @return uint16_t The number of Bytes sent
	 */
	static void SendMRP(struct cream::MRP_FRAME_HDR* dataHDR, std::vector<struct TRIGGER_RAW_HDR*>& triggers);

	static void Initialize();

	static uint16_t lastSentBytes;

	static ThreadsafeQueue<struct TRIGGER_RAW_HDR*>* multicastMRPQueues;
	static ThreadsafeQueue<unicastTriggerAndCrateCREAMIDs_type>* unicastMRPWithIPsQueues;

	static struct cream::MRP_FRAME_HDR* CREAM_MulticastRequestHdr;
	static struct cream::MRP_FRAME_HDR* CREAM_UnicastRequestHdr;

	static uint64_t L1TriggersSent;
	static std::atomic<uint64_t> L1MRPsSent;

	static bool paused;
};

} /* namespace cream */
} /* namespace na62 */
#endif /* L1DISTRIBUTIONHANDLER_H_ */
