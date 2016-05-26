/*#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <typeinfo>
#include <cmath>
#include <atomic>
#include <boost/timer/timer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/container/string.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include "structs/TriggerResponse.h"
#include "structs/EventID.h"
#include "utils/AExecutable.h"
#include "options/Logging.h"*/

#include "SharedMemoryManager.h"

namespace na62 {
uint SharedMemoryManager::l1_mem_size_;
uint SharedMemoryManager::l2_mem_size_;
uint SharedMemoryManager::to_q_size_;
uint SharedMemoryManager::from_q_size_;

boost::interprocess::managed_shared_memory* SharedMemoryManager::l1_shm_;
boost::interprocess::managed_shared_memory* SharedMemoryManager::l2_shm_;

boost::interprocess::message_queue * SharedMemoryManager::trigger_queue_;
boost::interprocess::message_queue * SharedMemoryManager::trigger_response_queue_;

void SharedMemoryManager::initialize(){

	l1_mem_size_ = 2;
	l2_mem_size_ = 10;
	to_q_size_ = 10;
	from_q_size_ = 10;


	//Pick the nearest multiple page size in byte per excess
	uint page_multiple_in_bytes = (l1_mem_size_ * sizeof(l1_SerializedEvent) /512 + 1 ) * 512;

	try {
		//in bytes
		l1_shm_ = new boost::interprocess::managed_shared_memory(boost::interprocess::create_only, "l1_shm", page_multiple_in_bytes);
	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what());
		l1_shm_ = new boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create, "l1_shm", page_multiple_in_bytes);
	}

	try {
		l2_shm_ = new boost::interprocess::managed_shared_memory(boost::interprocess::create_only, "l2_shm", page_multiple_in_bytes);
	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what());
		l2_shm_ = new boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create, "l2_shm", page_multiple_in_bytes);
	}

	try {
		trigger_queue_ = new boost::interprocess::message_queue(boost::interprocess::create_only, "trigger_queue", to_q_size_, sizeof(TriggerMessager));
	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what());
		trigger_queue_ = new boost::interprocess::message_queue(boost::interprocess::open_or_create, "trigger_queue", to_q_size_, sizeof(TriggerMessager));
	}

	try {
		trigger_response_queue_ = new boost::interprocess::message_queue(boost::interprocess::create_only, "trigger_response_queue", from_q_size_, sizeof(TriggerMessager));
	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what());
		trigger_response_queue_ = new boost::interprocess::message_queue(boost::interprocess::open_or_create, "trigger_response_queue", from_q_size_, sizeof(TriggerMessager));
	}
}

bool SharedMemoryManager::storeL1Event(uint event_id, Event event){
	std::pair<l1_SerializedEvent*, std::size_t> l1_d;
    l1_d = l1_shm_->find<l1_SerializedEvent>(label(event_id));

    if( !l1_d.first ){

    	TriggerMessager trigger_message;
    	trigger_message.id = event_id;
    	trigger_message.level = 1;

    	uint message_priority = 0;

		l1_SerializedEvent temp_serialized_event = serializeL1Event(event);
		l1_shm_->construct<l1_SerializedEvent>(label(event_id))(temp_serialized_event);

  	   //Enqueue Data
  	   //=============
  	    while( !trigger_queue_->try_send(&trigger_message, sizeof(TriggerMessager), message_priority) ){
  	  	  //TODO sleep for a while
  	    }
  	    //LOG_INFO("Sended event id: "<<ev->id<<" for l"<<ev->level<<" processing");

  	    return true;
    } else {
    	LOG_WARNING("Event: "<< event_id << "already in the memory!");
    	return false;
    }
}



bool SharedMemoryManager::removeL1Event(uint event_id){
	//Delete Event from Memory
	//=========================
	return l1_shm_->destroy<l1_SerializedEvent>(label(event_id));
}




bool SharedMemoryManager::popQueue(bool is_trigger_message_queue, TriggerMessager &trigger_message, uint &priority) {
	std::size_t struct_size = sizeof(TriggerMessager);
	std::size_t recvd_size;
	boost::interprocess::message_queue * queue;

	if (is_trigger_message_queue) {
		queue = trigger_queue_;
	} else {
		queue = trigger_response_queue_;
	}
	if (queue->try_receive((void *) &trigger_message, struct_size, recvd_size, priority)) {
		//Check than is the expected type
		if( recvd_size == struct_size ) {
			return true;
		}
		LOG_ERROR("Unexpected queue message received recvd side: "<<recvd_size<<" Instead of: "<<struct_size);
	}
	return false;
}

bool SharedMemoryManager::popTriggerQueue(TriggerMessager &trigger_message, uint &priority) {
	return popQueue(1, trigger_message, priority);
}

bool SharedMemoryManager::popTriggerResponseQueue(TriggerMessager &trigger_message, uint &priority) {
	return popQueue(0, trigger_message, priority);
}




bool SharedMemoryManager::pushTriggerResponseQueue(TriggerMessager trigger_message) {
	uint priority = 0;
	while( !trigger_response_queue_->try_send(&trigger_message, sizeof(TriggerMessager), priority) ){
			//usleep(10);
	}
	return true;
}


bool SharedMemoryManager::getNextEvent(Event &event, TriggerMessager & trigger_message) {
	//TriggerMessage temp_trigger_message;
	uint temp_priority;

	if (popTriggerQueue(trigger_message, temp_priority)) {
		if( trigger_message.level == 1 ){
			std::pair<l1_SerializedEvent*, std::size_t> l1_d;

			l1_d = l1_shm_->find<l1_SerializedEvent>(label(trigger_message.id));
			if ( l1_d.first != 0 ) {
				event = unserializeEvent(*l1_d.first);
                return true;
			}
			LOG_ERROR("Error couldn't find this piece of data in l1...");
			return false;

		}else if( trigger_message.level == 2 ){
			std::pair<l2_SerializedEvent*, std::size_t> l2_d;

			l2_d = l2_shm_->find<l2_SerializedEvent>(label(trigger_message.id));
			if ( l2_d.first != 0 ) {
				event = unserializeEvent(*l2_d.first);
				return true;
			}
			LOG_ERROR("Error couldn't find this piece of data in l2...");
			return false;
		}
		LOG_ERROR("No idea of which level you want to process..");
		return false;
	}
	return false;
}


//Producing Labels for l1_Events
//============================
char* SharedMemoryManager::label(uint n){
	const char* std_ID = "event_1024";
	const char* std_ID_format = "event_%04d";

	//TODO possible not use a new
	char* ID = new char[sizeof(std_ID)];
	std::sprintf(ID, std_ID_format, n);
	return ID;
}

}
