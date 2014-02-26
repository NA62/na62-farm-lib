/*
 * ZMQHandler.cpp
 *
 *  Created on: Feb 21, 2014
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#include "ZMQHandler.h"

#include <boost/thread/locks.hpp>
#include <unistd.h>
#include <zmq.hpp>
#include <iostream>
#include <map>
#include <sstream>

namespace na62 {

zmq::context_t ZMQHandler::context_;
std::set<std::string> ZMQHandler::boundAddresses_;
boost::mutex ZMQHandler::connectMutex_;

void ZMQHandler::Initialize() {
	zmq::context_t context_(1);
}

zmq::socket_t* ZMQHandler::GenerateSocket(int socketType) {
	return new zmq::socket_t(context_, socketType);
}

std::string ZMQHandler::GetEBL0Address(int threadNum) {
	std::stringstream address;
	address << "inproc://EB/" << threadNum << "/L0";
	return address.str();
}

std::string ZMQHandler::GetEBLKrAddress(int threadNum) {
	std::stringstream address;
	address << "inproc://EB/" << threadNum << "/LKr";
	return address.str();
}

void ZMQHandler::BindInproc(zmq::socket_t* socket, std::string address) {
	socket->bind(address.c_str());

	boost::lock_guard<boost::mutex> lock(connectMutex_);
	boundAddresses_.insert(address);
}

void ZMQHandler::ConnectInproc(zmq::socket_t* socket, std::string address) {
	connectMutex_.lock();
	while (boundAddresses_.find(address) == boundAddresses_.end()) {
		connectMutex_.unlock();
		usleep(100000);
		connectMutex_.lock();
	}
	socket->connect(address.c_str());
	connectMutex_.unlock();
}

} /* namespace na62 */