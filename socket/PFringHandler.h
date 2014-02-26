/*
 * PFringHandler.h
 *
 *  Created on: Jan 10, 2012
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#pragma once
#ifndef PFRINGHANDLER_H_
#define PFRINGHANDLER_H_

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <atomic>

#include "EthernetUtils.h"
#include "../utils/Utils.h"
#include "../utils/ThreadsafeQueue.h"
#include "../utils/Stopwatch.h"
#include "PFring.h" // BUGFIX: must be included AFTER any boost-header

namespace na62 {
class PFringHandler {
public:
	static void Initialize(std::string deviceName);

	static void Shutdown();

	static void StartARPThread();

	static inline int GetNextPacket(struct pfring_pkthdr *hdr, char** pkt,
			u_int pkt_len, uint8_t wait_for_incoming_packet, uint queueNumber) {
		int result = queueRings_[queueNumber]->get_next_packet(hdr, pkt,
				pkt_len, wait_for_incoming_packet);
		if (result == 1) {
			bytesReceived_++;
			packetsReceived_ += hdr->len;
		}

		return result;
	}

	static inline std::string GetDeviceName() {
		return deviceName_;
	}

	static inline int SendPacket(char *pkt, u_int pktLen, bool flush = true,
			bool activePoll = true) {
		/*
		 * Check if an Ethernet trailer is needed
		 */
		if (pktLen < 64) {
			memset(pkt + pktLen, 0, 64 - pktLen);
			pktLen = 64;
		}
		boost::lock_guard<boost::mutex> lock(sendMutex_); // Will lock sendMutex until return
		return queueRings_[1]->send_packet((char*) pkt, pktLen, flush,
				activePoll);
	}

	static inline int SendPacketConcurrently(uint16_t threadNum, char *pkt,
			u_int pktLen, bool flush = true, bool activePoll = true) {
		/*
		 * Check if an Ethernet trailer is needed
		 */
		if (pktLen < 64) {
			memset(pkt + pktLen, 0, 64 - pktLen);
			pktLen = 64;
		}

		if (numberOfQueues_ > 1) {
			return queueRings_[threadNum % numberOfQueues_]->send_packet(
					(char*) pkt, pktLen, flush, activePoll);
		} else {
			boost::lock_guard<boost::mutex> lock(sendMutex_); // Will lock sendMutex until return
			int result = queueRings_[0]->send_packet((char*) pkt, pktLen, flush,
					activePoll);
			return result;
		}
	}

	/**
	 * Returns the 6 byte long hardware address of the NIC the PFring object is assigned to.
	 */
	static inline char* GetMyMac() {
		return EthernetUtils::GetMacOfInterface(PFringHandler::GetDeviceName());
	}

	/**
	 * Returns the 4 byte long IP address of the NIC the PFring object is assigned to in network byte order.
	 */
	static inline u_int32_t GetMyIP() {
		return EthernetUtils::GetIPOfInterface(PFringHandler::GetDeviceName());
	}

	static inline uint64_t GetBytesReceived() {
		return bytesReceived_;
	}

	static inline uint64_t GetPacksReceived() {
		pfring_stat pfringStat = GetStats();
		return pfringStat.recv;
	}

	static void PrintStats();

	static void UpdateStats();

	static inline uint64_t GetPacksDroppedWorker() {

		pfring_stat pfringStat = GetStats();
		return pfringStat.drop;
	}

	static uint16_t GetNumberOfQueues() {
		return numberOfQueues_;
	}

private:
	static std::atomic<uint64_t> bytesReceived_;
	static std::atomic<uint64_t> packetsReceived_;
	static boost::mutex sendMutex_;
	static ntop::PFring ** queueRings_; // one ring per queue
	static ntop::PFring * mainReceiverRing_; // ring which receives IP packets
	static uint16_t numberOfQueues_;
	static std::string deviceName_;

	static boost::thread* ARPThread;

	static pfring_stat GetStats() {
		pfring_stat stats = { 0 };
		pfring_stat result = { 0 };
		for (int i = 0; i < numberOfQueues_; i++) {
			queueRings_[i]->get_stats(&stats);
			result.recv += stats.recv;
			result.drop += stats.drop;
		}
		return stats;
	}
}
;

} /* namespace na62 */
#endif /* PFRINGHANDLER_H_ */
