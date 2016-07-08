/*
 * MEPFragment.h
 *
 *  Created on: Mar 2, 2016
 *      Author: giovanna
 */

#ifndef L1MEPFRAGMENT_H_
#define L1MEPFRAGMENT_H_

#include <boost/noncopyable.hpp>
#include <netinet/in.h>
#include <cstdint>
#include "../exceptions/NA62Error.h"
#include "../eventBuilding/SourceIDManager.h"

namespace na62 {
namespace l1 {
class MEP;

struct L1_EVENT_RAW_HDR {
        uint32_t eventNumber :24;
        uint8_t sourceID;
        uint16_t numberOf4BWords;
        uint16_t reserved;
        uint32_t timestamp;
        uint16_t sourceSubID;
        uint8_t l0TriggerWord;
        uint8_t reserved2;
}__attribute__ ((__packed__));


class MEPFragment {
public:
	MEPFragment(MEP* mep, const L1_EVENT_RAW_HDR * data);
	~MEPFragment();

    inline const uint32_t getEventLength() const {
            return dataLength_;
    }

    /**
     * Absolute event number (MSB & LSB)
     */
    inline const uint32_t getEventNumber() const {
    	return rawData_->eventNumber;
    }

    inline const uint8_t getSourceID() const {
    	return rawData_->sourceID;
    }

	inline const uint32_t getEvent4BWords() const {
		return rawData_->numberOf4BWords;
	}

	inline const uint32_t getTimestamp() const {
		return rawData_->timestamp;
	}

	inline const uint16_t getSourceSubID() const {
		return rawData_->sourceSubID;
	}

	inline const uint8_t getL0TriggerWord() const {
		return rawData_->l0TriggerWord;
	}
    /**
     * Returns a pointer to the LKR-Buffer at the position where the data of this event starts (excluding header!).
     * From there on you should read only getEventLength()-sizeof(struct L1_EVENT_RAW_HDR) bytes!
     */
    inline const char* getData() const {
            return (((char*)rawData_) + sizeof(L1_EVENT_RAW_HDR));
    }

	/**
	 * Returns a pointer to the Buffer at the position where the data of this event starts (including header!).
	 * From there on you should read only getEventLength() bytes!
	 */
	inline const char* getDataWithHeader() const {
		return (const char*)rawData_;
	}

	inline const MEP* getMep() const {
		return mep_;
	}

	inline uint_fast8_t getSourceIDNum() const {
		return SourceIDManager::l1SourceIDToNum(rawData_->sourceID);
	}
private:

	const L1_EVENT_RAW_HDR * rawData_;
	uint16_t dataLength_;
	MEP* mep_;
};

} /* namespace l1 */
} /* namespace na62 */


#endif /* L1MEPFRAGMENT_H_ */
