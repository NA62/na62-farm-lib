/*
 * BurstIdHandler.cpp
 *
 *  Created on: May 27, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "BurstIdHandler.h"

#include "../options/Logging.h"
#include "../eventBuilding/Event.h"
#include "../eventBuilding/EventPool.h"
#include "../eventBuilding/SourceIDManager.h"

namespace na62 {
boost::timer::cpu_timer BurstIdHandler::EOBReceivedTimer_;
uint BurstIdHandler::nextBurstId_;
uint BurstIdHandler::currentBurstID_;
uint BurstIdHandler::lastFinishedBurst_=-1;
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

			std::cerr << "Unfinished event " << event->getEventNumber() << ": "
					<< std::endl;
			std::cerr << "\tMissing L0: " << std::endl;
			for (auto& sourceIDAndSubIds : event->getMissingSourceIDs()) {
				std::cerr << "\t"
						<< SourceIDManager::sourceIdToDetectorName(
								sourceIDAndSubIds.first) << ":" << std::endl;

				for (auto& subID : sourceIDAndSubIds.second) {
					std::cerr << "\t\t" << subID << ", ";
				}
				std::cerr << std::endl;
			}
			std::cerr << std::endl;

			std::cerr << "\tMissing CREAMs (crate: cream IDs): " << std::endl;
			for (auto& crateAndCreams : event->getMissingCreams()) {
				std::cerr << "\t\t" << crateAndCreams.first << ":\t";
				for (auto& creamID : crateAndCreams.second) {
					std::cerr << creamID << "\t";
				}
				std::cerr << std::endl;
			}
			std::cerr << std::endl;
		}
	}
}

} /* namespace na62 */
