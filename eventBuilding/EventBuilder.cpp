/*
 * EventCollector.cpp
 *
 *  Created on: Dec 6, 2011
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#include "EventBuilder.h"

#include <zmq.h>
#include <zmq.hpp>
#include <iostream>

#include "../socket/ZMQHandler.h"
#include "../utils/Utils.h"

namespace na62 {
EventBuilder::EventBuilder() :
		L0Socket_(ZMQHandler::GenerateSocket(ZMQ_PULL)), LKrSocket_(
				ZMQHandler::GenerateSocket(ZMQ_PULL)) {

}

EventBuilder::~EventBuilder() {
	delete L0Socket_;
	delete LKrSocket_;
}

void EventBuilder::thread() {
	ZMQHandler::BindInproc(L0Socket_, ZMQHandler::GetEBL0Address(threadNum_));
	ZMQHandler::BindInproc(LKrSocket_, ZMQHandler::GetEBLKrAddress(threadNum_));

	int timeout = 0;
	while (true) {
		L0Socket_->setsockopt(ZMQ_RCVTIMEO, &timeout, sizeof(timeout));

		zmq::message_t msg;
		if (L0Socket_->recv(&msg)) {
			Utils::PrintHex((char*) msg.data(), 4);
			std::cout << msg.data() << "????" << std::endl;
			timeout = 0;
		} else if (LKrSocket_->recv(&msg)) {
			timeout = 0;
		} else {
			std::cout << "timeout " << std::endl;
		}
	}

}

} /* namespace na62 */
