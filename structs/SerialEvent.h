/*
 * SerialEvent.h
 *
 *  Created on: May 30, 2016
 *      Author: Marco Boretto, Adam Peason
 */

#ifndef SERIALEVENT_H_
#define SERIALEVENT_H_

struct SerialEventHeader {
        uint length;
        uint event_id;
};


struct EventTest {
        uint event_id;
        char* data;
        uint length;
};

typedef std::array< char, 32768 > l1_SerializedEvent; //byte

#endif /* SERIALEVENT_H_ */



