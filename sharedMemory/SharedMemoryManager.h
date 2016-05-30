/*

#include <cstdlib>
#include <unistd.h>
#include <time.h>
#include <typeinfo>
#include <cmath>
#include <atomic>
#include <boost/timer/timer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/container/string.hpp>


#include <boost/interprocess/allocators/allocator.hpp>
#include "structs/TriggerResponse.h"

#include "utils/AExecutable.h"
 */

#include <iostream>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

#include <cstdlib>
#include <cstring>
#include <array>

#include "structs/TriggerMessager.h"
#include "options/Logging.h"
#include "structs/SerialEvent.h"

//TODO move somewhere not here
//typedef uint32_t Event;
//typedef uint32_t l1_SerializedEvent;
//typedef uint64_t l2_SerializedEvent;

//typedef char* Event;
//typedef char* l1_SerializedEvent;
//typedef char* l2_SerializedEvent;

typedef std::array< char, 128 > l1_SerializedEvent;

//using namespace boost::interprocess;
namespace na62 {

class SharedMemoryManager {
private:
	static uint l1_mem_size_;
	static uint l2_mem_size_;
	static uint to_q_size_;
	static uint from_q_size_;

	static boost::interprocess::managed_shared_memory *l1_shm_;
	static boost::interprocess::managed_shared_memory *l2_shm_;

	static boost::interprocess::message_queue *trigger_queue_;
	static boost::interprocess::message_queue *trigger_response_queue_;

	static char* label(uint n);


public:
	static void initialize();

	static inline boost::interprocess::managed_shared_memory * getL1SharedMemory() {
		return l1_shm_;
	}

	static inline boost::interprocess::managed_shared_memory * getL2SharedMemory() {
		return l2_shm_;
	}

	static inline boost::interprocess::message_queue * getTriggerQueue() {
		return trigger_queue_;
	}

	static inline boost::interprocess::message_queue * getTriggerResponseQueue() {
		return trigger_response_queue_;
	}

	//static inline bool cleanL1SharedMemory() {
	//	return TODO;
	//}

	static inline bool eraseL1SharedMemory() {
		try {
			return boost::interprocess::shared_memory_object::remove("l1_shm");
		} catch(boost::interprocess::interprocess_exception& e) {
			LOG_ERROR(e.what());
			return false;
		}
	}

	static inline bool eraseL2SharedMemory() {
		try {
			return boost::interprocess::shared_memory_object::remove("l2_shm");
		} catch(boost::interprocess::interprocess_exception& e) {
			LOG_ERROR(e.what());
			return false;
		}
	}

	static inline bool eraseTriggerQueue() {
		try {
			return boost::interprocess::message_queue::remove("trigger_queue");
		} catch(boost::interprocess::interprocess_exception& e) {
			LOG_ERROR(e.what());
			return false;
		}
	}

	static inline bool eraseTriggerResponseQueue() {
		try {
			return boost::interprocess::message_queue::remove("trigger_response_queue");
		} catch(boost::interprocess::interprocess_exception& e) {
			LOG_ERROR(e.what());
			return false;
		}
	}

	static inline void eraseAll() {
		eraseL1SharedMemory();
		eraseL2SharedMemory();
		eraseTriggerQueue();
		eraseTriggerResponseQueue();
	}

	static bool storeL1Event(Event temp_event);
	static bool removeL1Event(uint event_id);
	static bool getL1Event(uint event_id, Event &event);

	static bool popQueue(bool is_trigger_message_queue, TriggerMessager &trigger_message, uint &priority);
	static bool popTriggerQueue(TriggerMessager &trigger_message, uint &priority);
	static bool popTriggerResponseQueue(TriggerMessager &trigger_message, uint &priority);

	static bool pushTriggerResponseQueue(TriggerMessager trigger_message);


	static bool getNextEvent(Event &event, TriggerMessager &trigger_message);

	//static bool l1Tol2(TriggerMessager trigger_message);

	//Serialization and Unserialization
	//==================================
	//Will be moved


	static inline l1_SerializedEvent serializeL1Event(Event event) {
	        l1_SerializedEvent seriale;
	        SerialEventHeader header;
	        header.length = event.length;
	        header.event_id = event.event_id;

	        if (event.length > sizeof(l1_SerializedEvent)){
	                std::cout<<"error"<<std::endl;
	        }
	        memcpy((char*)&seriale, (char*)&header, sizeof(header));
	        memcpy(((char*) &seriale) + sizeof(header),  event.data, event.length);
	        //memcpy(((char*) &seriale) + 32,  (char*) event, event_size);
	        LOG_INFO("Length "<< header.length);

	        return seriale; // "array" being boost::array or std::array
	}

	static inline Event unserializeL1Event( l1_SerializedEvent &seriale) {

	        SerialEventHeader* header;
	        //header = (SerialEventHeader*) &seriale;
	        header = reinterpret_cast<SerialEventHeader*> (&seriale);
	        //hdr = reinterpret_cast<UDP_HDR*>(container.data);

	        //size = header->length;
	        std::cout<< "Length/ID "<< header->length<< "  " << header->event_id<<std::endl;
	        Event event;
	        event.event_id = header->event_id;
	        event.length = header->length;
	        event.data = new char[header->length];
	        memcpy(event.data,((char*) &seriale) + sizeof(header), header->length);
	        //memcpy(event,((char*) &seriale) , header->length);

	        return event; // "array" being boost::array or std::array
	}


};

}
/*

static std::pair<l2_SerializedEvent*, std::size_t> l2_d;



// Functions
//==========


static char* label(char *s){
  return label(atoi(s));
}







//L1 to L2 Conversion
//====================
static l2_Event l1tol2(l1_Event event){
  return (l2_Event) event;
}

 */

/*
 * L1 trigger function
 */
//static bool computeL1Trigger(l1_Event event);



/*
 * L2 trigger function
 */
//static bool computeL2Trigger(l2_Event event);

