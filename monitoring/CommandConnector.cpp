/*
 * CommandConnector.cpp
 *
 *  Created on: Jul 25, 2012
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#include <boost/asio.hpp>
#include "../eventBuilding/EventBuilder.h"
#include "CommandConnector.h"

namespace na62 {

using namespace boost::interprocess;

CommandConnector::CommandConnector() {
	LOG(INFO)<< "Starting CommandConnector";
	try {
		// Erase previous message queue
		message_queue::remove("command");
		// Open a message queue.
		commandQueue_.reset(new boost::interprocess::message_queue(create_only//only create
						, "command"//name
						, 100// max message number
						, 1024 * 64//max message size
				));
	} catch (interprocess_exception &ex) {
		message_queue::remove("command");
		LOG(ERROR) << "Unable to create command message queue: " << ex.what();
		boost::system::error_code noError;
	}
}

CommandConnector::~CommandConnector() {
	message_queue::remove("command");
}

void CommandConnector::thread() {
	unsigned int priority;
	message_queue::size_type recvd_size;

	std::string message;
	while (true) {
		message.resize(1024 * 64);

		/*
		 * Synchronious receive:
		 */
		commandQueue_->receive(&(message[0]), message.size(), recvd_size,
				priority);
		message.resize(recvd_size);

		LOG(INFO)<<"Received command: " << message;
		std::transform(message.begin(), message.end(), message.begin(),
				::tolower);

		std::vector<std::string> strings;
		boost::split(strings, message, boost::is_any_of(":"));
		if (strings.size() != 2) {
			LOG(ERROR)<<"Unknown command: " << message;
		} else {
			std::string command = strings[0];
			if (command == "updateburstid") {
				uint32_t burst = boost::lexical_cast<int>(strings[1]);
				LOG(INFO) << "Updating burst to " << burst;
//				EventBuilder::SetNextBurstID(burst);
			} else if (command == "sob_timestamp") {
				uint32_t timestamp = boost::lexical_cast<int>(strings[1]);
				EventBuilder::SetSOBtimestamp_(timestamp);
			}
		}
	}
}

}
/* namespace na62 */
