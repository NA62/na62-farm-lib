/*
 * Subevent.h
 *
 *  Created on: Jan 25, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef SUBEVENT_H_
#define SUBEVENT_H_

#include <boost/noncopyable.hpp>
#include <cstdint>

namespace na62 {
namespace l0 {
class MEPEvent;
} /* namespace l0 */
} /* namespace na62 */

namespace na62 {
namespace l0 {

class Subevent: private boost::noncopyable {
public:
	Subevent(const uint16_t expectedPacketsNum);
	virtual ~Subevent();

	void destroy();
	void inline addEventPart(MEPEvent* eventPart) throw () {
		eventParts[eventPartCounter++] = eventPart;
	}

	MEPEvent ** getEventParts() throw () {
		return eventParts;
	}

	MEPEvent* getPart(uint16_t eventPartNumber) throw () {
		return eventParts[eventPartNumber];
	}

	uint16_t inline getNumberOfParts() {
		return eventPartCounter;
	}

private:
	uint16_t ExpectedPacketsNum;
	MEPEvent ** eventParts;
	uint16_t eventPartCounter;
};

} /* namespace l0 */
} /* namespace na62 */
#endif /* SUBEVENT_H_ */
