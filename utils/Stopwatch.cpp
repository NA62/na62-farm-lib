/*
 * Stopwatch.hpp
 *
 *  Created on: Apr 21, 2011
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "Stopwatch.h"

namespace na62 {

uint64_t Stopwatch::cpuFrequency = 0;
uint64_t Stopwatch::ticksForOneUSleep = 0;

}
