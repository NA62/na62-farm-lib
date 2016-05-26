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

#include "structs/TriggerMessager.h"
#include "options/Logging.h"

//TODO move somewhere not here
typedef uint32_t Event;
typedef uint32_t l1_SerializedEvent;
typedef uint64_t l2_SerializedEvent;


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

	static bool storeL1Event(uint event_id_to_process, Event temp_event);
	static bool removeL1Event(uint event_id);

	static bool popQueue(bool is_trigger_message_queue, TriggerMessager &trigger_message, uint &priority);
	static bool popTriggerQueue(TriggerMessager &trigger_message, uint &priority);
	static bool popTriggerResponseQueue(TriggerMessager &trigger_message, uint &priority);

	static bool pushTriggerResponseQueue(TriggerMessager trigger_message);


	static bool getNextEvent(Event &event, TriggerMessager &trigger_message);

	//Serialization and Unserialization
	//==================================
	//Will be moved
	static inline l1_SerializedEvent serializeL1Event(Event event) {
	  //Just do noting here now, will be implement the logic for serialize a farm event
	  return (l1_SerializedEvent) event;
	}

	static inline l2_SerializedEvent serializeL2Event(Event event) {

	  return (l2_SerializedEvent) event;
	}

	static inline Event unserializeEvent(l1_SerializedEvent serialized_event) {
	   //Just do noting here now, will be implement the logic for unserialize a farm event
	   return (Event) serialized_event;
	}

	static inline Event unserializeEvent(l2_SerializedEvent serialized_event) {
	  return (Event) serialized_event;
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

