/*
 * BurstIdHandler.cpp
 *
 *  Created on: May 27, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "BurstIdHandler.h"

#include <sys/types.h>
#include <sstream>
#include <string>
#include <utility>

#include "../options/Logging.h"
#include "../eventBuilding/Event.h"
#include "../eventBuilding/EventPool.h"
#include "../eventBuilding/SourceIDManager.h"
#include "../utils/DataDumper.h"
#include "../l0/MEPFragment.h"
#include "../l0/Subevent.h"
#include "../structs/L0TPHeader.h"

namespace na62 {
boost::timer::cpu_timer BurstIdHandler::EOBReceivedTimer_;
uint BurstIdHandler::nextBurstId_;
uint BurstIdHandler::currentBurstID_;
uint BurstIdHandler::lastFinishedBurst_ = -1;
std::mutex BurstIdHandler::burstFinishedMutex_;
bool BurstIdHandler::resetCounter_ = false;

void BurstIdHandler::onBurstFinished() {
	int maxNumOfPrintouts = 100;

	int NL0ExpectedMEPs = SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT - 3; //L1-L2-NSTD packets
//	int NL0ExpectedMEPs = SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT;
	int NCREAMExpectedFragments =
			SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT;
//	for (uint eventNumber = 0;
//			eventNumber != EventPool::getPoolSize() + 1;
//			eventNumber++) {
//
//		if (EventPool::getL0PacketCounter()[eventNumber] != 0
//				&& EventPool::getL0PacketCounter()[eventNumber]
//						% NL0ExpectedMEPs != 0
//				|| EventPool::getCREAMPacketCounter()[eventNumber] != 0
//						&& EventPool::getCREAMPacketCounter()[eventNumber]
//								% NCREAMExpectedFragments != 0) {
//
//			if (EventPool::getL0PacketCounter()[eventNumber] < NL0ExpectedMEPs
//					|| EventPool::getCREAMPacketCounter()[eventNumber]
//							< NCREAMExpectedFragments) {
//				LOG_INFO<< " EventPool Unreconstructed location: " << eventNumber
//				<< " L0 MEPs:" << EventPool::getL0PacketCounter()[eventNumber] << "/" << NL0ExpectedMEPs
//				<< " CREAM Packets:" << EventPool::getCREAMPacketCounter()[eventNumber]<< "/" << NCREAMExpectedFragments << ENDL;
//			} else {
//				LOG_INFO<< " EventPool Fail location: " << eventNumber
//				<< " L0 MEPs:" << EventPool::getL0PacketCounter()[eventNumber] << "/" << NL0ExpectedMEPs
//				<< " CREAM Packets:" << EventPool::getCREAMPacketCounter()[eventNumber]<< "/" << NCREAMExpectedFragments << ENDL;
//			}
//		}
//		//Reset
//		EventPool::getL0PacketCounter()[eventNumber] = 0;
//		EventPool::getCREAMPacketCounter()[eventNumber] = 0;
//	}

	for (uint eventNumber = 0;
			eventNumber != EventPool::getLargestTouchedEventnumber() + 1;
			eventNumber++) {

		Event* event = EventPool::getEvent(eventNumber);
//		for (auto& sourceIDSubIds : event->getReceivedSourceIDsSourceSubIds()) {
//			LOG_INFO<< " +++++++++++MAP.first " << SourceIDManager::sourceIdToDetectorName(sourceIDSubIds.first) << ":" << ENDL;
//			for (auto& subId : sourceIDSubIds.second) {
//				LOG_INFO << "\t" << subId.first << ", " << (uint)subId.second << ENDL;
//			}
//			LOG_INFO << ENDL;
//		}
		if (event->isUnfinished()) {
			if (maxNumOfPrintouts-- == 0) {
				break;
			}

			std::stringstream dump;
			/*
			 * Print the global event timestamp and trigger word taken from the reference detector
			 */
			l0::MEPFragment* tsFragment = event->getL0SubeventBySourceIDNum(
					SourceIDManager::TS_SOURCEID_NUM)->getFragment(0);

			l0::MEPFragment* L0TPEvent = event->getL0TPSubevent()->getFragment(
					0);
			L0TpHeader* L0TPData = (L0TpHeader*) L0TPEvent->getPayload();

			dump << "Unfinished event " << event->getEventNumber()
					<< " burstID " << (uint) getCurrentBurstId() << " with TS "
					<< std::hex << tsFragment->getTimestamp()
					<< " and Trigword " << (uint) L0TPData->l0TriggerType
					<< std::dec << ": " << std::endl;

			dump << "\tMissing L0: " << std::endl;
			for (auto& sourceIDAndSubIds : event->getMissingSourceIDs()) {
				dump << "\t"
						<< SourceIDManager::sourceIdToDetectorName(
								sourceIDAndSubIds.first) << ":" << std::endl;

				for (auto& subID : sourceIDAndSubIds.second) {
					dump << "\t" << subID << ", ";
				}
				dump << std::endl;
			}
			dump << std::endl;

			if (event->isL1Processed()) {
				dump << "\tMissing CREAMs (crate: cream IDs): " << std::endl;
				for (auto& crateAndCreams : event->getMissingCreams()) {
					dump << "\t\t" << crateAndCreams.first << ":\t";
					for (auto& creamID : crateAndCreams.second) {
						dump << creamID << "\t";
					}
					dump << std::endl;
				}
				dump << std::endl;
			}
			DataDumper::printToFile("unfinishedEvents", "/tmp/farm-logs",
					dump.str());
			EventPool::freeEvent(event);
		}
	}
}

} /* namespace na62 */
