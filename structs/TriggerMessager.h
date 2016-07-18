/*
 * EventID.h
 *
 *  Created on: May 18, 2016
 *      Author: Adam Pearson
 */

#ifndef TRIGGER_MESSAGER_H_
#define TRIGGER_MESSAGER_H_

//#include <stdint.h>

struct TriggerMessager {
	uint memory_id;
	uint event_id;
	uint level;
	uint_fast8_t l1_trigger_type_word;
	bool trigger_result;
};

#endif /* EVENTID_H_ */

