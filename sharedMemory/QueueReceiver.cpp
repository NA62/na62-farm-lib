/*
 * QueueReceiver.cpp
 *
 *  Created on: May 16, 2016
 *      Author: marco
 */
#include "SharedMemoryManager.h"
#include "QueueReceiver.h"
#include "structs/TriggerMessager.h"
#include "structs/SerialEvent.h"


namespace na62 {


//Constructor and Destructor
//===========================

QueueReceiver::QueueReceiver(){

	running_ = true;
}

QueueReceiver::~QueueReceiver(){}



//Threading
//==========

void QueueReceiver::thread() {
	//All Those variable can became part of the class
	TriggerMessager trigger_message;
	uint priority = 0;

	while (running_) {


		//Receiving Response
		//=================
		if (SharedMemoryManager::popTriggerResponseQueue(trigger_message, priority)) {

			LOG_INFO("Queue Receiver Received event: "<<trigger_message.event_id);

			if (trigger_message.trigger_result) {

				LOG_INFO("event is good");

				//L1 Processing
				//==============
				if (trigger_message.level == 1){

					SharedMemoryManager::removeL1Event(trigger_message.memory_id);

					//LOG_INFO(" -> Sending L1 request for event id: "<< response->event_id);

				} else {
					LOG_INFO("Bad Level trigger to execute");
					continue;
				}

			} else {
				LOG_INFO("event is bad");

				SharedMemoryManager::removeL1Event(trigger_message.memory_id);
			}

		} else {
			boost::this_thread::sleep(boost::posix_time::microsec(50));
		}

		//usleep(1000000);
	}
}

void QueueReceiver::onInterruption() {
	running_ = false;
}
}
