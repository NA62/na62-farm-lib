#include "SharedMemoryManager.h"
#include "structs/Event.h"
#include "storage/SmartEventSerializer.h"

namespace na62 {

/*
 * Shared memory structures
 */
uint SharedMemoryManager::l1_shared_memory_fragment_size_;

uint SharedMemoryManager::l1_mem_size_;
uint SharedMemoryManager::l1_num_events_;
uint SharedMemoryManager::from_q_size_;

boost::interprocess::managed_shared_memory* SharedMemoryManager::l1_shm_; constexpr char SharedMemoryManager::l1_shm_name_[];
l1_SerializedEvent * SharedMemoryManager::l1_mem_array_; constexpr char SharedMemoryManager::l1_mem_array_name_[];

boost::interprocess::message_queue * SharedMemoryManager::trigger_queue_; constexpr char SharedMemoryManager::trigger_queue_name_[];
boost::interprocess::message_queue * SharedMemoryManager::trigger_response_queue_; constexpr char SharedMemoryManager::trigger_response_queue_name_[];
boost::interprocess::message_queue * SharedMemoryManager::l1_free_queue_; constexpr char SharedMemoryManager::l1_free_queue_name_[];

/*
 * Stats Counters
 */
std::atomic<uint64_t> SharedMemoryManager::FragmentStored_(0);
std::atomic<uint64_t> SharedMemoryManager::FragmentNonStored_(0);

//stats send receive
std::map<uint_fast32_t, std::pair<std::atomic<	int64_t>, std::atomic<int64_t>>> SharedMemoryManager::l1_event_counter_;
std::map<uint_fast32_t, std::pair<std::atomic<	int64_t>, std::atomic<int64_t>>> SharedMemoryManager::l1_request_stored_;

void SharedMemoryManager::initialize(){

	l1_shared_memory_fragment_size_= sizeof(l1_SerializedEvent);

	//l1_mem_size_ = 100000000;  //in bytes relationship: num events < l1_mem_size/sizeof(l1_SerializedEvent) (by a small percent)
	l1_mem_size_ = 500000000;  //in bytes relationship: num events < l1_mem_size/sizeof(l1_SerializedEvent) (by a small percent)


	//This is an initial value estimated from the size of the memory and the size of the structure
	//Can be decreased during the initialization of the big array
	l1_num_events_ = l1_mem_size_ /l1_shared_memory_fragment_size_; //bytes

	//Please notice that that can't hold more events than the memory array
	//to_q_size_ = 1000000;
	from_q_size_ = 1000000;


	//Initailizing array
	try {
		l1_shm_ = new boost::interprocess::managed_shared_memory(boost::interprocess::create_only, l1_shm_name_, l1_mem_size_);//in bytes
	} catch(boost::interprocess::interprocess_exception& e) {
		LOG_INFO(e.what()<< "L1 exists");
		l1_shm_ = new boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create, l1_shm_name_, l1_mem_size_);
	}
	if (!getL1MemArray()) {
		if (!createL1MemArray( l1_num_events_ )) {
			LOG_ERROR("L1 Mem Array Creation Error");
		}
	} else {
		l1_mem_array_ = getL1MemArray();
		LOG_INFO("L1 Mem Array Already Created");
	}
	LOG_INFO("Shared memory L1 in bytes: " << l1_mem_size_ << " Number of fragments available: " << getL1NumEvents());


	//initializing free queue
	try {
		l1_free_queue_ = new boost::interprocess::message_queue(boost::interprocess::create_only, l1_free_queue_name_, getL1NumEvents() , sizeof(uint));

		LOG_INFO("Pushing all free indices onto l1_free_queue_");
		//Filling free fragments
		fillFreeQueue();
	} catch (boost::interprocess::interprocess_exception& ex) {
		LOG_INFO(ex.what()<< "L1 Free Queue exists");
		l1_free_queue_ = new boost::interprocess::message_queue(boost::interprocess::open_or_create, l1_free_queue_name_, getL1NumEvents() , sizeof(uint));
	}

	//send queue
	try {
		trigger_queue_ = new boost::interprocess::message_queue(boost::interprocess::create_only, trigger_queue_name_, getL1NumEvents(), sizeof(TriggerMessager));
	} catch (boost::interprocess::interprocess_exception& ex) {
		LOG_INFO(ex.what()<< "Trigger Queue exists");
		trigger_queue_ = new boost::interprocess::message_queue(boost::interprocess::open_or_create, trigger_queue_name_, getL1NumEvents(), sizeof(TriggerMessager));
	}

	//receive queue
	try {
		trigger_response_queue_ = new boost::interprocess::message_queue(boost::interprocess::create_only, trigger_response_queue_name_, from_q_size_, sizeof(TriggerMessager));
	} catch (boost::interprocess::interprocess_exception& ex) {
		LOG_INFO(ex.what()<< "Trigger Response Queue exists");
		trigger_response_queue_ = new boost::interprocess::message_queue(boost::interprocess::open_or_create, trigger_response_queue_name_, from_q_size_, sizeof(TriggerMessager));
	}
}

//L1 Shared Memory Functions
bool SharedMemoryManager::storeL1Event(const Event* event) {
	//Retrieving free memory space
	uint memory_id;
	while ( !SharedMemoryManager::popL1FreeQueue(memory_id) ) {
		//TODO need to slow down the request??? Can i also call recursively myself
		LOG_ERROR(" No free memory available...");
	}

	//LOG_INFO("Attempting to Store Event "<< event->getEventNumber() <<" at "<<memory_id);
	TriggerMessager trigger_message;
	trigger_message.memory_id = memory_id;
	trigger_message.event_id = event->getEventNumber();
	trigger_message.burst_id = event->getBurstID();
	trigger_message.level = 1;

	uint message_priority = 0;

	try {
		EVENT_HDR* smartserializedevent = SmartEventSerializer::SerializeEvent(event, l1_mem_array_ + memory_id);
		FragmentStored_.fetch_add(1, std::memory_order_relaxed);
		//std::cout<<"Serialized on shared memory!!!!"<<std::endl;

		//Enqueue Data
		while (1) {
			for (int i = 0; i < 100; i++) {
				if (trigger_queue_->try_send(&trigger_message, sizeof(TriggerMessager), message_priority)) {
					return true;
				}
			}
			LOG_WARNING("Tried pushing on trigger queue 100 times, now waiting");
			usleep(1000);
		}

	} catch(SerializeError &) {
		LOG_ERROR("Fragment exceed the memory! "<< "Shared Memory buffer size: " << getL1SharedMemoryFragmentSize());
		removeL1Event(memory_id);
		FragmentNonStored_.fetch_add(1, std::memory_order_relaxed);
		return false;
	}

	LOG_ERROR("How did we get here??");
	return false;
}

bool SharedMemoryManager::removeL1Event(uint memory_id){
	if (pushL1FreeQueue(memory_id)) {
		return true;
	}
	LOG_ERROR("Unable to push on the free event queue");
	return false;
}

bool SharedMemoryManager::getNextEvent(Event* & event, TriggerMessager & trigger_message) {
	uint priority = 0;

	if (popTriggerQueue(trigger_message, priority)) {
		//LOG_INFO("Getting Event at "<< trigger_message.memory_id);

		event = new Event( (EVENT_HDR*) (l1_mem_array_ + trigger_message.memory_id), 1);
		return true;
	}
	return false;
}

//Queue Functions
//================
bool SharedMemoryManager::popTriggerQueue(TriggerMessager &trigger_message, uint &priority) {
	return popQueue(1, trigger_message, priority);
}

bool SharedMemoryManager::popTriggerResponseQueue(TriggerMessager &trigger_message, uint &priority) {
	return popQueue(0, trigger_message, priority);
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

bool SharedMemoryManager::pushTriggerResponseQueue(TriggerMessager &trigger_message) {
	uint priority = 0;

	while (1){
		for (int i = 0; i < 100; i++) {
			if (trigger_response_queue_->try_send(&trigger_message, sizeof(TriggerMessager), priority)) {
				return true;
			}
			LOG_WARNING("Tried pushing on trigger response queue 100 times, now waiting");
			usleep(1000);
		}
	}
	return true;
}

bool SharedMemoryManager::popL1FreeQueue(uint &memory_id) {
	std::size_t recvd_size;
	uint priority;

	if (l1_free_queue_->try_receive((void *) &memory_id, sizeof(uint), recvd_size, priority)) {
		//Check that is the expected type
		if (recvd_size == sizeof(uint)) {
			return true;
		}
		LOG_ERROR("Unexpected l1_free_queue_ recvd side: "<<recvd_size<<" Instead of: "<<sizeof(uint));
	}
	return false;
}

bool SharedMemoryManager::pushL1FreeQueue(uint memory_id) {
	uint priority = 0;

	while (1) {
		for (int i = 0; i < 100; i++) {
			if (l1_free_queue_->try_send(&memory_id, sizeof(uint), priority)) {
				return true;
			}
		}
		LOG_WARNING("Tried pushing on l1_free_queue_ 100 times, now waiting");
		usleep(1000);
	}
	return true;
}

}
