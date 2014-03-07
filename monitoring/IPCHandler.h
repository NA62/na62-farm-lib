/*
 * IPCHandler.h
 *
 *  Created on: Nov 26, 2012
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#pragma once
#ifndef IPCHANDLER_H_
#define IPCHANDLER_H_

#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <string>

namespace na62 {
typedef boost::shared_ptr<boost::interprocess::message_queue> message_queue_ptr;

enum STATE {
	// 0=IDLE; 1=INITIALIZED; 2=RUNNING; Other=ERROR
	OFF,
	INITIALIZED,
	RUNNING,
	ERROR
};

class IPCHandler {
public:

	static void updateState(STATE newState);
	static void sendErrorMessage(std::string Message);
	static void sendStatistics(std::string name, std::string values);

private:
	static void connectToDIMInterface();

	static STATE currentState;

	static message_queue_ptr stateQueue_;
	static message_queue_ptr statisticsQueue_;
};

} /* namespace na62 */
#endif /* IPCHANDLER_H_ */
