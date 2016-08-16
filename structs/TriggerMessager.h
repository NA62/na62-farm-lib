/*
 * EventID.h
 *
 *  Created on: May 18, 2016
 *      Author: Adam Pearson
 */

#ifndef TRIGGER_MESSAGER_H_
#define TRIGGER_MESSAGER_H_

struct TriggerMessager {
	uint memory_id;
	uint_fast32_t event_id;
	uint_fast32_t burst_id;
	uint level;
	uint_fast8_t l1_trigger_type_word; //Filled from the trigger processor
	bool trigger_result;
};

#endif /* EVENTID_H_ */

