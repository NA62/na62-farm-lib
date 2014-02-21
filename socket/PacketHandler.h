/*
 * PacketHandler.h
 *
 *  Created on: Feb 7, 2012
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#pragma once
#ifndef PACKETHANDLER_H_
#define PACKETHANDLER_H_

#include <vector>
#include "../utils/AExecutable.h"

namespace zmq {
	class socket_t;
}
namespace na62 {
struct DataContainer;

class PacketHandler: public AExecutable {
public:
	PacketHandler();
	virtual ~PacketHandler();

private:
	void processPacket(DataContainer container);
	void thread();
	void connectZMQ();
	std::vector<zmq::socket_t*> EBL0sockets_;
	std::vector<zmq::socket_t*> EBLKrSockets_;
};

} /* namespace na62 */
#endif /* PACKETHANDLER_H_ */
