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
uint SharedMemoryManager::l1_mem_size_; uint SharedMemoryManager::l1_num_events_;
uint SharedMemoryManager::l2_mem_size_;
uint SharedMemoryManager::to_q_size_;
uint SharedMemoryManager::from_q_size_;


boost::interprocess::managed_shared_memory* SharedMemoryManager::l1_shm_;
boost::interprocess::managed_shared_memory* SharedMemoryManager::l2_shm_;

boost::interprocess::message_queue * SharedMemoryManager::trigger_queue_;
boost::interprocess::message_queue * SharedMemoryManager::trigger_response_queue_;
boost::interprocess::message_queue * SharedMemoryManager::l1_free_queue_;


/*
uint SharedMemoryManager::getL1NumEvents(){
	return l1_num_events_;
}

void SharedMemoryManager::setL1NumEvents( uint num ){
	l1_num_events_ = num;
}*/

void SharedMemoryManager::initialize(){

	l1_mem_size_ = 10000; //bytes
	l2_mem_size_ = 10;
	to_q_size_ = 1000000;
	from_q_size_ = 1000000;


	//Pick the nearest multiple page size in byte per excess
	uint page_multiple_in_bytes = (l1_mem_size_ * sizeof(l1_SerializedEvent) /512 + 1 ) * 512 * 2 ;
	//uint page_multiple_in_bytes = l1_mem_size_;

	/*
		sizeof(l1_SerializedEvent) = 128
	    l1_mem_size_  ->  number of events
	    10000 -> 50
	    100000 -> 519
		1000000 -> 5207
		10000000 -> 52082
		100000000 -> 520832
		1000000000 -> 5208332

		sizeof(l1_SerializedEvent) = 256
		l1_mem_size_  ->  number of events
	    10000 -> 30
	    100000 -> 311
		1000000 -> 3124
   	   	10000000 -> 31249
		100000000 -> 312499
		1000000000 -> 3124999

		sizeof(l1_SerializedEvent) = 1000
		10000 -> 9
		100000 -> 94
		1000000 -> 946
		10000000 -> 9469
		100000000 -> 94696
		1000000000 -> 946969

		relationship: num events < l1_mem_size/sizeof(l1_SerializedEvent) (by a small percent)
	*/
	LOG_INFO("Shared memory 1 in bytes: " << l1_mem_size_);

	try {
		//in bytes
		l1_shm_ = new boost::interprocess::managed_shared_memory(boost::interprocess::create_only, "l1_shm_", l1_mem_size_);

		//Set Aside Blocks for Data
		l1_SerializedEvent temp_serialized_event;
		for(uint memory_id = 0; ; memory_id ++){
			try{
				l1_shm_->construct<l1_SerializedEvent>(label(memory_id))(temp_serialized_event);
				//l1_shm_->construct<l1_SerializedEvent>(i)(temp_serialized_event);
			}
			catch(boost::interprocess::interprocess_exception& e){
				//Memory filled
				SharedMemoryManager::setL1NumEvents(memory_id );
				LOG_WARNING("L1 SHM filled with "<< memory_id  <<" events");
				break;
			}
		}

	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what()<< "L1 exists");
		l1_shm_ = new boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create, "l1_shm_", l1_mem_size_);
	}

	try {
		l1_free_queue_ = new boost::interprocess::message_queue(boost::interprocess::create_only, "l1_free_queue_", SharedMemoryManager::getL1NumEvents() , sizeof(uint));

		LOG_INFO("Pushing all free indices onto l1_free_queue_");

		for( uint memory_id = 0; memory_id  < SharedMemoryManager::getL1NumEvents(); memory_id++ ) SharedMemoryManager::pushL1FreeQueue(memory_id);

	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what()<< "L1 Free Queue exists");
		l1_free_queue_ = new boost::interprocess::message_queue(boost::interprocess::open_or_create, "l1_free_queue_", SharedMemoryManager::getL1NumEvents() , sizeof(uint));
	}



	try {
		l2_shm_ = new boost::interprocess::managed_shared_memory(boost::interprocess::create_only, "l2_shm_", page_multiple_in_bytes);
	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what()<< "L2 exists");
		l2_shm_ = new boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create, "l2_shm_", page_multiple_in_bytes);
	}

	try {
		trigger_queue_ = new boost::interprocess::message_queue(boost::interprocess::create_only, "trigger_queue_", to_q_size_, sizeof(TriggerMessager));
	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what()<< "Trigger Queue exists");
		trigger_queue_ = new boost::interprocess::message_queue(boost::interprocess::open_or_create, "trigger_queue_", to_q_size_, sizeof(TriggerMessager));
	}

	try {
		trigger_response_queue_ = new boost::interprocess::message_queue(boost::interprocess::create_only, "trigger_response_queue_", from_q_size_, sizeof(TriggerMessager));
	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what()<< "Trigger Response Queue exists");
		trigger_response_queue_ = new boost::interprocess::message_queue(boost::interprocess::open_or_create, "trigger_response_queue_", from_q_size_, sizeof(TriggerMessager));
	}

}

bool SharedMemoryManager::storeL1Event(Event event){

	uint memory_id;
	//Rretrieving free memory space
	if ( ! SharedMemoryManager::popL1FreeQueue(memory_id)) {
		LOG_ERROR(" No free memory available...");
		return false;
	}
	LOG_INFO("Attempting to Store Event "<< event.event_id <<" at "<<memory_id);

	std::pair<l1_SerializedEvent*, std::size_t> l1_d;
	l1_d = l1_shm_->find<l1_SerializedEvent>(label(memory_id));
	//l1_d = l1_shm_->find<l1_SerializedEvent>(event.event_id);


	if( l1_d.first ){
		TriggerMessager trigger_message;
		trigger_message.memory_id = memory_id;
		trigger_message.event_id = event.event_id;
		trigger_message.level = 1;

		uint message_priority = 0;

		SharedMemoryManager::serializeL1Event(event, l1_d.first);

		/*
    		try {
    			l1_shm_->construct<l1_SerializedEvent>(label(event.event_id))(temp_serialized_event);
    			LOG_INFO("Event: inserted in the memory!... ");
    		} catch(boost::interprocess::interprocess_exception& e) {
    			LOG_INFO("Failed to store L1 event... "<<e.what());
    			return false;
    		}
		 */

		//Enqueue Data
		//=============
		while( 1 ){
			for( int i = 0; i < 100; i++ )
				if( trigger_queue_->try_send(&trigger_message, sizeof(TriggerMessager), message_priority) ) return true;
			LOG_WARNING("Tried pushing on trigger queue 100 times, now waiting");
			usleep(1000);
		}

		return true;
	} else {
		LOG_ERROR(memory_id << " is not a valid memory_id...");
		return false;
	}
}


bool SharedMemoryManager::removeL1Event(uint memory_id){
	//Delete Event from Memory
	//=========================
	//return l1_shm_->destroy<l1_SerializedEvent>(label(event_id));
	if ( SharedMemoryManager::pushL1FreeQueue(memory_id)) {
		return true;
	}
	LOG_ERROR("Unable to push on the free event queue");
	return false;
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
		//Check that is the expected type
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

	while( 1 ){
		for(int i = 0; i < 100; i++)
			if( trigger_response_queue_->try_send(&trigger_message, sizeof(TriggerMessager), priority) ) return true;
		LOG_WARNING("Tried pushing on trigger response queue 100 times, now waiting");
	  	usleep(1000);
	}
	return true;
}




bool SharedMemoryManager::popL1FreeQueue(uint &memory_id){
	std::size_t recvd_size;
	uint priority;

	if (l1_free_queue_->try_receive((void *) &memory_id, sizeof(uint), recvd_size, priority)) {
		//Check that is the expected type
		if( recvd_size == sizeof(uint) ) {
			return true;
		}
		LOG_ERROR("Unexpected l1_free_queue_ recvd side: "<<recvd_size<<" Instead of: "<<sizeof(uint));
	}
	return false;

}



bool SharedMemoryManager::pushL1FreeQueue(uint memory_id) {
	uint priority = 0;

	while( 1 ){
		for(int i = 0; i < 100; i++)
			if( l1_free_queue_->try_send(&memory_id, sizeof(uint), priority) ) return true;
		LOG_WARNING("Tried pushing on l1_free_queue_ 100 times, now waiting");
	  	usleep(1000);
	}
	return true;
}


bool SharedMemoryManager::getNextEvent(Event &event, TriggerMessager & trigger_message) {
	//TriggerMessage temp_trigger_message;
	uint temp_priority;

	if (popTriggerQueue(trigger_message, temp_priority)) {
		if( trigger_message.level == 1 ){

			std::pair<l1_SerializedEvent*, std::size_t> l1_d;

			try {
				l1_d = l1_shm_->find<l1_SerializedEvent>(label(trigger_message.memory_id));
				LOG_INFO("Memory index: "<<trigger_message.memory_id);
				//l1_d = l1_shm_->find<l1_SerializedEvent>(trigger_message.id);
			} catch(boost::interprocess::interprocess_exception& e) {
				LOG_INFO(e.what());
			}

			if ( l1_d.first != 0 ) {
				event = SharedMemoryManager::unserializeL1Event(*l1_d.first);
                return true;
			}
			LOG_ERROR("Error couldn't find this piece of data in l1...");
			return false;

		}
		/*else if( trigger_message.level == 2 ){
			std::pair<l2_SerializedEvent*, std::size_t> l2_d;

			l2_d = l2_shm_->find<l2_SerializedEvent>(label(trigger_message.id));
			if ( l2_d.first != 0 ) {
				event = unserializeEvent(*l2_d.first);
				return true;
			}
			LOG_ERROR("Error couldn't find this piece of data in l2...");
			return false;
		}*/
		LOG_ERROR("No idea of which level you want to process..");
		return false;
	}

	return false;
}


//Producing Labels for l1_Events
//============================
char* SharedMemoryManager::label(uint memory_id){
	//TODO integer labels instead of char* possible?
    const char* std_ID = "10000000000";
	const char* std_ID_format = "%04d";

	//TODO possible not use a new
	char* ID = new char[sizeof(std_ID)];
	std::sprintf(ID, std_ID_format, memory_id);
	return ID;
}




}
