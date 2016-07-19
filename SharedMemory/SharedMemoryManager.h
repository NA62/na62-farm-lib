#ifndef SHAREDMEMOPRYMANAGER_H_
#define SHAREDMEMOPRYMANAGER_H_

#include <iostream>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

#include <cstdlib>
#include <string>
#include <array>
#include <atomic>

#include "structs/TriggerMessager.h"
#include "options/Logging.h"
#include "structs/SerialEvent.h"
#include "structs/Event.h"

#include <eventBuilding/Event.h>
#include "exceptions/SerializeError.h"

/*
 * Mean l2 serialized event size : 16.7Kb
 * Min l2 serialized event size : 15.4 Kb
 * Max  l2 serialized event size : 24.4 Kb
 *
 *
 * L2 sources ~ 500
 * L1 Sources ~ 70
 *
 * Mean L1 serialized event size ~  2 Kb
 */

namespace na62 {

class SharedMemoryManager {
private:
	/*
	 * Shared memory structures
	 */
	static uint l1_shared_memory_fragment_size_;

	static uint l1_mem_size_;
	static uint l1_num_events_;

	static uint from_q_size_;

	static boost::interprocess::managed_shared_memory *l1_shm_; static constexpr char l1_shm_name_[] = "l1_shm_";
	static l1_SerializedEvent *l1_mem_array_; static constexpr char l1_mem_array_name_[] = "l1_mem_array_";

	static boost::interprocess::message_queue *trigger_queue_; static constexpr char trigger_queue_name_[] = "trigger_queue_";
	static boost::interprocess::message_queue *trigger_response_queue_; static constexpr char trigger_response_queue_name_[] = "trigger_response_queue_";
	static boost::interprocess::message_queue *l1_free_queue_; static constexpr char l1_free_queue_name_[] = "l1_free_queue_";

	/*
	 * Stats Counters
	 */
	static std::atomic<uint64_t> FragmentStored_;
	static std::atomic<uint64_t> FragmentNonStored_;

	//Create the big array to store serialized data
	//Attemp recursively to decrease the number of data to store if the initialization don't succeed
	static inline bool createL1MemArray(uint size){
		uint difference_factor = 1;
		l1_SerializedEvent temp_event;
		try{
			l1_mem_array_ = l1_shm_->construct<l1_SerializedEvent>(l1_mem_array_name_)[size](temp_event);
			setL1NumEvents(size);
			return true;
		} catch( boost::interprocess::interprocess_exception& e) {
			//Assuming that the attempted size was the issue...
			LOG_WARNING(size<<" is too big for l1_mem_array_... trying "<< size/difference_factor);
			return createL1MemArray( size - difference_factor );
		}
		return false;
	}
	static inline void setL1NumEvents( uint num ){
		l1_num_events_ = num;
	}

	static bool popL1FreeQueue(uint &memory_id);
	static bool pushL1FreeQueue(uint memory_id);
	static bool popQueue(bool is_trigger_message_queue, TriggerMessager &trigger_message, uint &priority);
	static bool popTriggerQueue(TriggerMessager &trigger_message, uint &priority);

public:

	static void initialize();

	static inline uint getL1NumEvents(){
		return l1_num_events_;
	}

	static inline uint  getL1SharedMemoryFragmentSize() {
			return l1_shared_memory_fragment_size_; // In bytes
	}

	static inline boost::interprocess::managed_shared_memory * getL1SharedMemory() {
		return l1_shm_;
	}

	static inline boost::interprocess::message_queue * getTriggerQueue() {
		return trigger_queue_;
	}

	static inline boost::interprocess::message_queue * getTriggerResponseQueue() {
		return trigger_response_queue_;
	}

	static inline l1_SerializedEvent* getL1MemArray(){
		return l1_shm_->find<l1_SerializedEvent>(l1_mem_array_name_).first;
	}

	static inline bool eraseL1SharedMemory() {
		try {
			return boost::interprocess::shared_memory_object::remove(l1_shm_name_);
		} catch(boost::interprocess::interprocess_exception& e) {
			LOG_ERROR(e.what());
			return false;
		}
	}

	static inline bool eraseTriggerQueue() {
		try {
			return boost::interprocess::message_queue::remove(trigger_queue_name_);
		} catch(boost::interprocess::interprocess_exception& e) {
			LOG_ERROR(e.what());
			return false;
		}
	}

	static inline bool eraseTriggerResponseQueue() {
		try {
			return boost::interprocess::message_queue::remove(trigger_response_queue_name_);
		} catch(boost::interprocess::interprocess_exception& e) {
			LOG_ERROR(e.what());
			return false;
		}
	}

	static inline bool eraseL1FreeQueue() {
		try {
			return boost::interprocess::message_queue::remove(l1_free_queue_name_);
		} catch(boost::interprocess::interprocess_exception& e) {
			LOG_ERROR(e.what());
			return false;
		}
	}

	static inline bool destroyL1MemArray(){
		try{
			if (l1_shm_) {
				return l1_shm_->destroy<l1_SerializedEvent>(l1_mem_array_name_);
			}
			return true;

		} catch(boost::interprocess::interprocess_exception& e) {
			LOG_ERROR(e.what()<<" l1_mem_array_ destroying error...");
			return false;
		}
	}

	static inline void eraseAll() {
		eraseL1SharedMemory();
		eraseTriggerQueue();
		eraseTriggerResponseQueue();
		eraseL1FreeQueue();
		destroyL1MemArray();
	}

	static bool storeL1Event(const Event* event);

	static bool getNextEvent(Event* & event, TriggerMessager & trigger_message);
	static bool removeL1Event(uint memory_id);

	static bool pushTriggerResponseQueue(TriggerMessager &trigger_message);
	static bool popTriggerResponseQueue(TriggerMessager &trigger_message, uint &priority);

	static inline float getStoreRatio() {
		return  ((float) FragmentStored_ / (float) (FragmentNonStored_ + FragmentStored_)) ;
	}

	//static bool storeL1Event(EventTest &temp_event);
	//Serialization and Unserialization just for testing with random data
	//==================================
/*	static inline void serializeL1Event(EventTest event, l1_SerializedEvent* seriale, uint max_length = -1){

		SerialEventHeader header;
		header.length = event.length;
		header.event_id = event.event_id;

		if ( max_length != -1 && event.length > max_length){ //l1_shared_memory_fragment_size_

			throw SerializeError("Fragmento is too big");

			//std::cout<<"error"<<std::endl;
		}

		memcpy((char*)seriale, (char*)&header, sizeof(header));
		memcpy(((char*)seriale) + sizeof(header),  event.data, event.length);

		LOG_INFO("Length "<< header.length);
	}

	static inline void unserializeL1Event(EventTest &event, l1_SerializedEvent* seriale) {

		SerialEventHeader* header;
		header = reinterpret_cast<SerialEventHeader*> (seriale);

		LOG_INFO("Length/ID "<< header->length<< "  " << header->event_id);

		event.event_id = header->event_id;
		event.length = header->length;
		event.data = new char[header->length];
		memcpy(event.data,((char*) seriale) + sizeof(header), header->length);
	}*/
};

}
#endif /* SHAREDMEMOPRYMANAGER_H_ */

