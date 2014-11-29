/*
 * UnfinishedEventsCollector.h
 *
 *  Created on: Nov 28, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef MONITORING_UNFINISHEDEVENTSCOLLECTOR_H_
#define MONITORING_UNFINISHEDEVENTSCOLLECTOR_H_

#include <sys/types.h>
#include <map>
#include <string>

namespace na62 {

class UnfinishedEventsCollector {
public:
	static void addReceivedSubSourceIdFromUnfinishedEvent(uint sourceNum,
			uint subSourceID);

	static std::string toJson();

private:
	static std::map<uint, std::map<uint, uint>> receivedEventsBySubsourceBySourceID;
};

} /* namespace na62 */

#endif /* MONITORING_UNFINISHEDEVENTSCOLLECTOR_H_ */
