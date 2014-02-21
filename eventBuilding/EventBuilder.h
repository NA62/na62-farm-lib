/*
 * EventCollector.h
 *
 *  Created on: Dec 6, 2011
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#pragma once
#ifndef EVENTBUILDER_H_
#define EVENTBUILDER_H_

#include "../utils/AExecutable.h"

namespace zmq {
class socket_t;
} /* namespace zmq */

namespace na62 {

class EventBuilder: public AExecutable {
public:
	EventBuilder();
	virtual ~EventBuilder();

private:
	zmq::socket_t* L0Socket_;
	zmq::socket_t* LKrSocket_;
	void thread();
};

} /* namespace na62 */
#endif /* EVENTBUILDER_H_ */
