#include "SharedMemoryManager.h"
#include "structs/Event.h"
#include "storage/SmartEventSerializer.h"

namespace na62 {


/*
 * Shared memory structures
 */

uint SharedMemoryManager::l1_shared_memory_fragment_size_;

uint SharedMemoryManager::l1_mem_size_; uint SharedMemoryManager::l1_num_events_;
uint SharedMemoryManager::l2_mem_size_;
uint SharedMemoryManager::to_q_size_;
uint SharedMemoryManager::from_q_size_;


boost::interprocess::managed_shared_memory* SharedMemoryManager::l1_shm_; constexpr char* SharedMemoryManager::l1_shm_name_;
boost::interprocess::managed_shared_memory* SharedMemoryManager::l2_shm_; constexpr char* SharedMemoryManager::l2_shm_name_;

boost::interprocess::message_queue * SharedMemoryManager::trigger_queue_; constexpr char* SharedMemoryManager::trigger_queue_name_;
boost::interprocess::message_queue * SharedMemoryManager::trigger_response_queue_; constexpr char* SharedMemoryManager::trigger_response_queue_name_;
boost::interprocess::message_queue * SharedMemoryManager::l1_free_queue_; constexpr char* SharedMemoryManager::l1_free_queue_name_;

l1_SerializedEvent * SharedMemoryManager::l1_mem_array_; //constexpr char* SharedMemoryManager::l1_mem_array_name_;


/*
 * Stats Counters
 */
std::atomic<uint64_t> SharedMemoryManager::FragmentStored_(0);
std::atomic<uint64_t> SharedMemoryManager::FragmentNonStored_(0);



void SharedMemoryManager::initialize(){


	l1_shared_memory_fragment_size_= sizeof(l1_SerializedEvent);

	l1_mem_size_ = 500000;  // relationship: num events < l1_mem_size/sizeof(l1_SerializedEvent) (by a small percent)
	l1_num_events_ = l1_mem_size_ /sizeof(l1_SerializedEvent); //bytes
	l2_mem_size_ = 10000;
	to_q_size_ = 1000000;
	from_q_size_ = 1000000;

	LOG_INFO("Shared memory 1 in bytes: " << l1_mem_size_);

	try {
		//in bytes
		l1_shm_ = new boost::interprocess::managed_shared_memory(boost::interprocess::create_only, l1_shm_name_, l1_mem_size_);
	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what()<< "L1 exists");
		l1_shm_ = new boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create, l1_shm_name_, l1_mem_size_);
	}

	if( !SharedMemoryManager::getL1MemArray() ){
		if ( !SharedMemoryManager::createL1MemArray( l1_num_events_ ) ) {
			LOG_ERROR("L1 Mem Array Creation Error");
		}
	} else {
		l1_mem_array_ = SharedMemoryManager::getL1MemArray();
		LOG_INFO("L1 Mem Array Already Created");
	}

	try {
		l2_shm_ = new boost::interprocess::managed_shared_memory(boost::interprocess::create_only, l2_shm_name_, l2_mem_size_);
	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what()<< "L2 exists");
		l2_shm_ = new boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create, l2_shm_name_, l2_mem_size_);
	}




	try {
		l1_free_queue_ = new boost::interprocess::message_queue(boost::interprocess::create_only, l1_free_queue_name_, SharedMemoryManager::getL1NumEvents() , sizeof(uint));

		LOG_INFO("Pushing all free indices onto l1_free_queue_");

		for( uint memory_id = 0; memory_id  < SharedMemoryManager::getL1NumEvents(); memory_id++ ) SharedMemoryManager::pushL1FreeQueue(memory_id);

	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what()<< "L1 Free Queue exists");
		l1_free_queue_ = new boost::interprocess::message_queue(boost::interprocess::open_or_create, l1_free_queue_name_, SharedMemoryManager::getL1NumEvents() , sizeof(uint));
	}

	try {
		trigger_queue_ = new boost::interprocess::message_queue(boost::interprocess::create_only, trigger_queue_name_, to_q_size_, sizeof(TriggerMessager));
	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what()<< "Trigger Queue exists");
		trigger_queue_ = new boost::interprocess::message_queue(boost::interprocess::open_or_create, trigger_queue_name_, to_q_size_, sizeof(TriggerMessager));
	}

	try {
		trigger_response_queue_ = new boost::interprocess::message_queue(boost::interprocess::create_only, trigger_response_queue_name_, from_q_size_, sizeof(TriggerMessager));
	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what()<< "Trigger Response Queue exists");
		trigger_response_queue_ = new boost::interprocess::message_queue(boost::interprocess::open_or_create, trigger_response_queue_name_, from_q_size_, sizeof(TriggerMessager));
	}

}


//L1 Shared Memory Functions
//===========================
bool SharedMemoryManager::storeL1Event(const Event* event) {
	//Retrieving free memory space
	uint memory_id;

	while ( !SharedMemoryManager::popL1FreeQueue(memory_id) ) {
		//TODO need to slow down the request??? Can i also call recursively myself
		LOG_ERROR(" No free memory available...");
	}
	LOG_INFO("Attempting to Store Event "<< event->getEventNumber() <<" at "<<memory_id);

	TriggerMessager trigger_message;
	trigger_message.memory_id = memory_id;
	trigger_message.event_id = event->getEventNumber();
	trigger_message.level = 1;

	uint message_priority = 0;

	try {
		//SharedMemoryManager::serializeL1Event(event, l1_mem_array_ + memory_id, l1_shared_memory_fragment_size_);
		EVENT_HDR* smartserializedevent = SmartEventSerializer::SerializeEvent(event, l1_mem_array_ + memory_id);
		FragmentStored_.fetch_add(1, std::memory_order_relaxed);
		//Enqueue Data
		//=============
		while (1) {
			for (int i = 0; i < 100; i++) {

				if (trigger_queue_->try_send(&trigger_message, sizeof(TriggerMessager), message_priority)) {

					return true;
				}
			}
			LOG_WARNING("Tried pushing on trigger queue 100 times, now waiting");
			usleep(1000);
		}

	} catch(SerializeError) {
		LOG_ERROR("Fragment exceed the memory");
//
//		removeL1Event(memory_id);
//		FragmentNonStored_.fetch_add(1, std::memory_order_relaxed);
//		return false;
	}

	LOG_ERROR("How did we get here??");
	return false;
}

bool SharedMemoryManager::storeL1Event(EventTest &event){
	//Retrieving free memory space
	uint memory_id;
	while ( !SharedMemoryManager::popL1FreeQueue(memory_id) ) {
		//TODO need to slow down the request??? Can i also call recursively myself
		LOG_ERROR(" No free memory available...");
	}

	LOG_INFO("Attempting to Store Event "<< event.event_id <<" at "<<memory_id);

	TriggerMessager trigger_message;
	trigger_message.memory_id = memory_id;
	trigger_message.event_id = event.event_id;
	trigger_message.level = 1;

	uint message_priority = 0;

	try {
		SharedMemoryManager::serializeL1Event(event, l1_mem_array_ + memory_id, l1_shared_memory_fragment_size_);
		FragmentStored_.fetch_add(1, std::memory_order_relaxed);


		//Enqueue Data
		//=============
		while(1) {
			for (int i = 0; i < 100; i++) {

				if (trigger_queue_->try_send(&trigger_message, sizeof(TriggerMessager), message_priority)) {
					return true;
				}
			}
			LOG_WARNING("Tried pushing on trigger queue 100 times, now waiting");
			usleep(1000);
		}

	} catch(SerializeError) {
		LOG_ERROR("Fragmet exeed the memory");

		removeL1Event(memory_id);
		FragmentNonStored_.fetch_add(1, std::memory_order_relaxed);
		return false;
	}

	LOG_ERROR("How did we get here??");
	return false;
}


bool SharedMemoryManager::removeL1Event(uint memory_id){

	if ( SharedMemoryManager::pushL1FreeQueue(memory_id)) {
		return true;
	}
	LOG_ERROR("Unable to push on the free event queue");
	return false;
}

bool SharedMemoryManager::getNextEvent(EventTest &event, TriggerMessager & trigger_message) {
	uint temp_priority;

	if (popTriggerQueue(trigger_message, temp_priority)) {
		if( trigger_message.level == 1 ){

			LOG_INFO("Getting Event at "<< trigger_message.memory_id);

			SharedMemoryManager::unserializeL1Event(event, l1_mem_array_ + trigger_message.memory_id);
			return true;
		}

		LOG_ERROR("No idea of which level you want to process..");
		return false;
	}

	return false;
}

//Queue Functions
//================
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

bool SharedMemoryManager::pushTriggerResponseQueue(TriggerMessager &trigger_message) {
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






/*
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
 */



}
