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
#include "../l0/MEPEvent.h"

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

	// LKrSocket should never block
	LKrSocket_->setsockopt(ZMQ_RCVTIMEO, &timeout, sizeof(timeout));

	while (true) {
		/*
		 * L0Socket will block if the last poll did not find a packet
		 * The timeout will be increased very time no packet was found
		 */
		L0Socket_->setsockopt(ZMQ_RCVTIMEO, &timeout, sizeof(timeout));

		zmq::message_t msg;

		try {

			if (L0Socket_->recv(&msg)) {
				l0::MEPEvent* event = (l0::MEPEvent*) msg.data();
				std::cerr << event->getEventNumber() << " received"
						<< std::endl;

				timeout = 0;
			} else if (LKrSocket_->recv(&msg)) {
				timeout = 0;
			} else {
				if (timeout == 0) {
					timeout = 1;
				} else {
					timeout *= 2;
					if (timeout > 1000) {
						timeout = -1;
					}
				}
			}

		} catch (const zmq::error_t& ex) {
			if (ex.num() != EINTR) {
				std::cerr << ex.what() << std::endl;
				return;
			}
		}

	}
}

} /* namespace na62 */
