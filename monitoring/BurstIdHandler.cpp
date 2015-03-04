/*
 * BurstIdHandler.cpp
 *
 *  Created on: May 27, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "BurstIdHandler.h"

namespace na62 {
boost::timer::cpu_timer BurstIdHandler::EOBReceivedTimer_;
uint BurstIdHandler::nextBurstId_;
uint BurstIdHandler::currentBurstID_;

} /* namespace na62 */
