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

namespace na62 {
boost::timer::cpu_timer BurstIdHandler::EOBReceivedTimer_;
uint BurstIdHandler::nextBurstId_;
uint BurstIdHandler::currentBurstID_;
uint BurstIdHandler::lastFinishedBurst_ = -1;
std::mutex BurstIdHandler::burstFinishedMutex_;

void BurstIdHandler::onBurstFinished() {
	int maxNumOfPrintouts = 100;

	for (uint eventNumber = 0;
			eventNumber != EventPool::getLargestTouchedEventnumber() + 1;
			eventNumber++) {

		Event* event = EventPool::getEvent(eventNumber);
		if (event->isUnfinished()) {
			if (maxNumOfPrintouts-- == 0) {
				break;
			}

			std::stringstream dump;

			dump << "Unfinished event " << event->getEventNumber()
					<< " with TS " << event->getTimestamp() << ": "
					<< std::endl;
			dump << "\tMissing L0: " << std::endl;
			for (auto& sourceIDAndSubIds : event->getMissingSourceIDs()) {
				dump << "\t"
						<< SourceIDManager::sourceIdToDetectorName(
								sourceIDAndSubIds.first) << ":" << std::endl;

				for (auto& subID : sourceIDAndSubIds.second) {
					dump << "\t\t" << subID << ", ";
				}
				dump << std::endl;
			}
			dump << std::endl;

			dump << "\tMissing CREAMs (crate: cream IDs): " << std::endl;
			for (auto& crateAndCreams : event->getMissingCreams()) {
				dump << "\t\t" << crateAndCreams.first << ":\t";
				for (auto& creamID : crateAndCreams.second) {
					dump << creamID << "\t";
				}
				dump << std::endl;
			}
			dump << std::endl;
			DataDumper::printToFile("unfinishedEvents", "/tmp/farm-logs",
					dump.str());
		}
	}
}

} /* namespace na62 */
