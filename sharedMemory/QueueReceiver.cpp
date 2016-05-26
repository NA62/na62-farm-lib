/*
 * QueueReceiver.cpp
 *
 *  Created on: May 16, 2016
 *      Author: marco
 */
#include "SharedMemoryManager.h"
#include "QueueReceiver.h"
#include "structs/TriggerMessager.h"

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
		SharedMemoryManager::removeL1Event(trigger_message.id);


/*
	  //Act on Result
	  //==============

	  //Event is Good
	  //==============
	  if (trigger_message.trigger_result) {

		//L1 Processing
		//==============
		if (trigger_message.level == 1){
		  //LOG_INFO(" -> Sending L1 request for event id: "<< response->event_id);

		  //Find and Destroy Event from L1 SHM and store in L2 SHM
		  //======================================================
		  l1_d = l1_shm->find<l1_SerializedEvent>(ID);

		  if( l1_d.first ){
			l1_shm->destroy<l1_SerializedEvent>(ID);
			l2_SerializedEvent temp_event = serializeEvent(l1tol2(*l1_d.first));

			l2_d.first = l2_shm->find_or_construct<l2_SerializedEvent>(ID)(temp_event);
			//Question: Any advantage (time/memory) in actually destroying this chunk of memory and constructing it with the new event? Should/Could the below method be used for general replacement of events (i.e. reduce number of function calls) with some sort of flag in each event saying it's ready to be deleted or perhaps a queue of event ID's ready to be deleted?
			//===========================================================
			if( l2_d.first ) *l2_d.first = temp_event;

		  } else {
			  continue;
		  }


		  //Sending Event to be Checked for L2 processing
		  //============================================
		  EventID *ev = new EventID;
		  ev->id = response->event_id;
		  ev->level = 2;

		  while( !toCheckQ_->try_send(ev, sizeof(EventID), priority) ){
		//usleep(10);
		  }
		  //LOG_INFO("Sended event id: "<<ev->id<<" for l"<<ev->level<<" processing");

		}else if (response->level == 2){
		  //LOG_INFO(" -> Sending L2 request for event id: "<< response->event_id);
		}

	  } else {
		//LOG_INFO(" -> Discard event id: "<< response->event_id << "from L"<< response->level << " triggering");

		//Delete Event from Memory
		//=========================
		if( !l1_shm->destroy<l1_SerializedEvent>(ID) )
		  l2_shm->destroy<l2_SerializedEvent>(ID);

	  }
*/
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
