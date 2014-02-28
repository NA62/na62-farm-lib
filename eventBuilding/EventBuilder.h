/*
 * EventCollector.h
 *
 *  Created on: Dec 6, 2011
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#pragma once
#ifndef EVENTBUILDER_H_
#define EVENTBUILDER_H_

#include <vector>

#include "../utils/AExecutable.h"

namespace na62 {
class Event;
namespace cream {
class LKREvent;
} /* namespace cream */
namespace l0 {
class MEPEvent;
} /* namespace l0 */
} /* namespace na62 */

namespace zmq {
class socket_t;
} /* namespace zmq */

namespace na62 {

class EventBuilder: public AExecutable {
public:
	EventBuilder();
	virtual ~EventBuilder();

private:
	void thread();

	void handleL0Data(l0::MEPEvent * mepEvent);
	void handleLKRData(cream::LKREvent * lkrEvent);

	void processL1(Event *event);
	void processL2(Event * event);


	zmq::socket_t* L0Socket_;
	zmq::socket_t* LKrSocket_;

	std::vector<Event*> eventPool;

	const int NUMBER_OF_EBS;

};

} /* namespace na62 */
#endif /* EVENTBUILDER_H_ */
