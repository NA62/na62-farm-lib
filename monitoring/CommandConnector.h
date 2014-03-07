/*
 * CommandConnector.h
 *
 *  Created on: Jul 25, 2012
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#pragma once
#ifndef COMMANDCONNECTOR_H_
#define COMMANDCONNECTOR_H_

#include <boost/noncopyable.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

#include "../utils/AExecutable.h"

namespace na62 {

class CommandConnector: private boost::noncopyable, public AExecutable  {
public:
	CommandConnector();
	virtual ~CommandConnector();
	void run();
private:
	void thread();

	typedef boost::shared_ptr<boost::interprocess::message_queue> message_queue_ptr;
	message_queue_ptr commandQueue_;
};

} /* namespace na62 */
#endif /* COMMANDCONNECTOR_H_ */
