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
//32768 This is too much a compete serialized events (L1 + L2) weight: 22Kbyte
//typedef std::array< char, 32768 > l1_SerializedEvent; //byte
typedef std::array< char, 24576 > l1_SerializedEvent; //byte
//typedef std::array< char, 16384 > l1_SerializedEvent; //byte

#endif /* SERIALEVENT_H_ */



