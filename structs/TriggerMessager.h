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
	uint id;
	uint level;
	bool trigger_result;
};

#endif /* EVENTID_H_ */

