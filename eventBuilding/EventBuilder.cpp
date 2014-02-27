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
	L0Socket_->close();
	LKrSocket_->close();
	delete L0Socket_;
	delete LKrSocket_;
}

void EventBuilder::thread() {
	ZMQHandler::BindInproc(L0Socket_, ZMQHandler::GetEBL0Address(threadNum_));
	ZMQHandler::BindInproc(LKrSocket_, ZMQHandler::GetEBLKrAddress(threadNum_));

	zmq::pollitem_t items[] = { { *L0Socket_, 0, ZMQ_POLLIN, 0 }, { *LKrSocket_,
			0, ZMQ_POLLIN, 0 } };

	while (1) {
		try {
			boost::this_thread::interruption_point();

			zmq::message_t message;
			zmq::poll(&items[0], 2, 1000); // Poll 1s to pass interruption_point

			if (items[0].revents & ZMQ_POLLIN) {
				L0Socket_->recv(&message);
				l0::MEPEvent* event = (l0::MEPEvent*) message.data();
				std::cerr << event->getEventNumber() << " received"
						<< std::endl;
			}
			if (items[1].revents & ZMQ_POLLIN) {
				LKrSocket_->recv(&message);
			}
		} catch (const zmq::error_t& ex) {
			if (ex.num() != EINTR) {
				L0Socket_->close();
				LKrSocket_->close();
				std::cerr << ex.what() << std::endl;
				return;
			}
		}
	}
}

} /* namespace na62 */
