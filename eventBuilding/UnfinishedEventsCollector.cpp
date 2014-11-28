/*
 * UnfinishedEventsCollector.cpp
 *
 *  Created on: Nov 28, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "UnfinishedEventsCollector.h"

namespace na62 {
std::map<uint, std::map<uint, uint>> UnfinishedEventsCollector::receivedEventsBySubsourceBySourceID;

} /* namespace na62 */
