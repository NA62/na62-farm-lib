/*
 * SerialEvent.h
 *
 *  Created on: May 30, 2016
 *      Author: Marco Boretto, Adams Peasona
 */

#ifndef SERIALEVENT_H_
#define SERIALEVENT_H_

struct SerialEventHeader {
        uint length;
        uint event_id;
};


struct Event {
        uint event_id;
        char* data;
        uint length;
};

#endif /* SERIALEVENT_H_ */



