/*
 * MEP.h
 *
 *  Created on: Mar 2, 2016
 *      Author: giovanna
 */

#ifndef L1MEP_H_
#define L1MEP_H_



#include <stdint.h>
#include <vector>
#include <atomic>
#include <boost/noncopyable.hpp>

#include "../exceptions/UnknownCREAMSourceIDFound.h"
#include "../exceptions/BrokenPacketReceivedError.h"
#include "../structs/DataContainer.h"
#include "../eventBuilding/SourceIDManager.h"
#include "MEPFragment.h"

namespace na62 {
namespace l1 {

class MEP: private boost::noncopyable  {
public:
        /**
         * Reads the data coming from L0 and initializes the corresponding fields
         */
        MEP(const char * data, const uint16_t& dataLength,
                        DataContainer originalData) ;

        /**
         * Frees the data buffer (orignialData) that was created by the Receiver
         *
         * Should only be called by ~MEPFragment() as a MEP may not be deleted until every MEPFragment is processed and deleted.
         */
        ~MEP();

        void initializeMEPFragments(const char* data, const uint16_t& dataLength);

        /**
         * Returns a pointer to the n'th event within this MEP where 0<=n<getFirstEventNum()
         */
        inline MEPFragment* getEvent(const uint16_t n) {
                /*
                 * n may be bigger than <getNumberOfEvents()> as <deleteEvent()> could have been invoked already
                 */
                return events[n];
        }

        inline uint16_t getNumberOfEvents() const {
                return eventNum_;
        }

    	/**
    	 * Returns the source ID of the detector that has sent this MEP
    	 */
    	inline uint_fast8_t getSourceID() const {
    		return sourceID_;
    	}

    	/**
    	 * This method is used to "plug holes" in the data source IDs. So if you have 3 sources being not in a row like {2, 5, 7}
    	 * you would probably want to have an array with three entries, one for each source. For this you need a relation like 2->0, 5->1, 7->2.
    	 * This is done by this method!
    	 */
    	inline uint_fast8_t getSourceIDNum() const {
    		return SourceIDManager::l1SourceIDToNum(sourceID_);
    	}

        /**
         * Returns true if no more events are remaining (all have been processed and sent/deleted).
         * So if true this MEP can also be deleted (together with its original UDP packet)
         *
         * This method is thread safe
         */
        inline bool deleteEvent() {

            /*
             * Decrement eventCount. If we reach 0 we can delete this object as all events have been processed.
             */
            return --eventNum_ == 0;
    }

        DataContainer getRawData() {
        	return dataContainer_;
        }

private:
    // The whole ethernet frame
    DataContainer dataContainer_;
    // Pointers to the payload of the UDP packet
     std::vector<MEPFragment*> events;
     std::atomic<int> eventNum_;
     uint_fast8_t sourceID_;


};

} /* namespace l1 */
} /* namespace na62 */



#endif /* L1MEP_H_ */
