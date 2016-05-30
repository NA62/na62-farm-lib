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

		//Receiving Response from Trigger Processor and Acting on It
		//===========================================================


		//Receiving Response
		//=================
		if (SharedMemoryManager::popTriggerResponseQueue(trigger_message, priority)) {

			LOG_INFO("Queue Receiver Received event: "<<trigger_message.id);

			//Act on Result
			//==============

			//Event is Good
			//==============
			if (trigger_message.trigger_result) {

				LOG_INFO("event is good");

				//L1 Processing
				//==============
				if (trigger_message.level == 1){


					//SharedMemoryManager::l1Tol2(trigger_message);
					//Event temp_event;
					//SharedMemoryManager::getL1Event(trigger_message.id, temp_event);
					//LOG_INFO("Event = "<< temp_event[0] << temp_event[1] << temp_event[2]);
					SharedMemoryManager::removeL1Event(trigger_message.id);

					//LOG_INFO(" -> Sending L1 request for event id: "<< response->event_id);

					} else {
						LOG_INFO("Bad Level trigger to execute");
						continue;
					}
/*

					//Sending Event to be Checked for L2 processing
					//============================================
					EventID *ev = new EventID;
					ev->id = response->event_id;
					ev->level = 2;

					while( !toCheckQ_->try_send(ev, sizeof(EventID), priority) ){
						//usleep(10);
					}
					//LOG_INFO("Sended event id: "<<ev->id<<" for l"<<ev->level<<" processing");

					 */
				/*
			}
				else if (response->level == 2){
					//LOG_INFO(" -> Sending L2 request for event id: "<< response->event_id);
				}
				*/

			} else {
				LOG_INFO("event is bad");

				SharedMemoryManager::removeL1Event(trigger_message.id);

				//LOG_INFO(" -> Discard event id: "<< response->event_id << "from L"<< response->level << " triggering");
/*
				//Delete Event from Memory
				//=========================
				if( !l1_shm->destroy<l1_SerializedEvent>(ID) )
					l2_shm->destroy<l2_SerializedEvent>(ID);

					*/

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
