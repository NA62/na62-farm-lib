/*
 * UnfinishedEventsCollector.h
 *
 *  Created on: Nov 28, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef MONITORING_UNFINISHEDEVENTSCOLLECTOR_H_
#define MONITORING_UNFINISHEDEVENTSCOLLECTOR_H_

#include <sys/types.h>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

namespace na62 {

class UnfinishedEventsCollector {
public:
	static void addReceivedSubSourceIdFromUnfinishedEvent(uint sourceNum,
			uint subSourceID) {
		auto lb = receivedEventsBySubsourceBySourceID.lower_bound(sourceNum);
		// Check if The sourceNum already exists
		if (lb != receivedEventsBySubsourceBySourceID.end()
				&& !(receivedEventsBySubsourceBySourceID.key_comp()(sourceNum,
						lb->first))) {
			/*
			 *	SourceNum exists -> check if subsourceID exists
			 */
			auto lb2 = lb->second.lower_bound(subSourceID);
			if (lb2 != lb->second.end()
					&& !(lb->second.key_comp()(sourceNum, lb->first))) {
				// SubsourceID exists -> increment it's value
				lb2->second++;
			} else {
				// create a new map entry for the subsourceID
				lb->second.insert(lb2,
						std::map<uint, uint>::value_type(subSourceID, 1));
			}

		} else {
			/*
			 * Nothing stored for this sourceID -> store new map
			 */
			std::map<uint, uint> subsourceAndData;
			subsourceAndData[subSourceID] = 1;
			receivedEventsBySubsourceBySourceID.insert(lb,
					std::map<uint, std::map<uint, uint>>::value_type(sourceNum,
							subsourceAndData));
		}
	}

	static std::string toJson() {
		std::stringstream stream;

		stream << "{";

		for (const auto& sourceAndData : receivedEventsBySubsourceBySourceID) {
			stream << sourceAndData.first << ":{";

			for (auto& subsourceAndEventNum : sourceAndData.second) {
				stream << subsourceAndEventNum.first << ":"
						<< subsourceAndEventNum.second;
				if (&subsourceAndEventNum != &*(--sourceAndData.second.end())) {
					stream << ",";
				}
			}

			stream << "}";

			if (&sourceAndData
					!= &*(--receivedEventsBySubsourceBySourceID.end())) {
				stream << ",";
			}
		}
		stream << "}";
		return stream.str();
	}

private:
	static std::map<uint, std::map<uint, uint>> receivedEventsBySubsourceBySourceID;
};

} /* namespace na62 */

#endif /* MONITORING_UNFINISHEDEVENTSCOLLECTOR_H_ */
