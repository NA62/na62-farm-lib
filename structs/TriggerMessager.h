/*
 * EventID.h
 *
 *  Created on: May 18, 2016
 *      Author: Adam Pearson
 */

#include "l1/L1InfoToStorage.h"

#ifndef TRIGGER_MESSAGER_H_
#define TRIGGER_MESSAGER_H_

struct TriggerMessager {
	uint memory_id;
	uint_fast32_t event_id;
	uint_fast32_t burst_id;
	uint level;
	uint_fast8_t l1_trigger_type_word; //Filled from the trigger processor

	//uint_fast8_t l1TriggerWords;
	std::array<uint_fast8_t, 16> l1TriggerWords; //Filled from the trigger processor
	L1InfoToStorage l1Info; //Filled from the trigger processor
	bool isL1WhileTimeout; //Filled from the trigger processor
	bool isRequestZeroSuppressed;

	bool trigger_result;
};

#endif /* EVENTID_H_ */

