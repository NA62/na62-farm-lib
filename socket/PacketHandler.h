/*
 * PacketHandler.h
 *
 *  Created on: Feb 7, 2012
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#pragma once
#ifndef PACKETHANDLER_H_
#define PACKETHANDLER_H_

#include "../utils/AExecutable.h"

namespace na62 {
struct DataContainer;

class PacketHandler: public AExecutable {
public:
	PacketHandler();
	virtual ~PacketHandler();

private:
	void processPacket(DataContainer container);
	/**
	 * May be run by only one single thread!
	 * It pops the queue <dataPool> and sequentially reads	 * the packets: MEP packets will be
	 *ructed and sent to the EventCollector object.
	 */
	void thread();
};

} /* namespace na62 */
#endif /* PACKETHANDLER_H_ */
