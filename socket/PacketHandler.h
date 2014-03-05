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
	/**
	 * @return <true> In case of success, false in case of a serious error (we should stop the thread in this case)
	 */
	bool processPacket(DataContainer container);
	void processARPRequest(struct ARP_HDR* arp);
	void thread();
	void connectZMQ();

	/**
	 * @return <true> If no checksum errors have been found
	 */
	bool checkFrame(struct UDP_HDR* hdr, uint16_t length);

	std::vector<zmq::socket_t*> EBL0sockets_;
	std::vector<zmq::socket_t*> EBLKrSockets_;
};

} /* namespace na62 */
#endif /* PACKETHANDLER_H_ */
