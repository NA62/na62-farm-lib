/*
 * IPCHandler.cpp
 *
 *  Created on: Nov 26, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "IPCHandler.h"

#include <boost/interprocess/creation_tags.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <unistd.h>
#include <string>
#include <glog/logging.h>

namespace na62 {

using namespace boost::interprocess;

STATE IPCHandler::currentState = OFF;
message_queue_ptr IPCHandler::stateQueue_;
message_queue_ptr IPCHandler::statisticsQueue_;

void IPCHandler::connectToDIMInterface() {
//	while (true) {
//		try {
//			stateQueue_.reset(new message_queue(open_only // only create
//					, "state" // name
//					));
//
//			statisticsQueue_.reset(new message_queue(open_only // only create
//					, "statistics" // name
//					));
//			return;
//		} catch (interprocess_exception &ex) {
//			stateQueue_.reset();
//			statisticsQueue_.reset();
//			LOG(ERROR)<< "Unable to connect to DIM interface program: "
//			<< ex.what();
//			sleep(5);
//		}
//	}
}

void IPCHandler::updateState(STATE newState) {
//	currentState = newState;
//
//	if (!stateQueue_) {
//		connectToDIMInterface();
//	}
//	try {
//		if (!stateQueue_->try_send(&currentState, sizeof(STATE), 0)) {
//			LOG(ERROR)<< "Unable to send message to DIM interface program!";
//			sleep(5);
//		}
//	} catch (interprocess_exception &ex) {
//		LOG(ERROR) << "Unable to send message to DIM interface program: "
//		<< ex.what();
//		stateQueue_.reset();
//	}
}

void IPCHandler::sendErrorMessage(std::string message) {
	sendStatistics("ErrorMessage", message);
}

void IPCHandler::sendStatistics(std::string name, std::string values) {
//	while (!stateQueue_) {
//		connectToDIMInterface();
//		usleep(1000);
//	}
//
//	if (!statisticsQueue_ || name.empty() || values.empty()) {
//		return;
//	}
//
//	std::string message = name + ":" + values;
//	try {
//		if (!statisticsQueue_->try_send(message.data(), message.length(), 0)) {
//			statisticsQueue_.reset();
//			LOG(ERROR)<< "Unable to send statistics to dim-service!";
//			sleep(5);
//		}
//	} catch (interprocess_exception &ex) {
//		LOG(ERROR)<<"Unable to send message to DIM interface program: "
//		<< ex.what();
//		stateQueue_.reset();
//	}
}

}
/* namespace na62 */
