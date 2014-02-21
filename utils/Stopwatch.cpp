/*
 * Stopwatch.hpp
 *
 *  Created on: Apr 21, 2011
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#include "Stopwatch.h"

namespace na62 {

uint64_t Stopwatch::cpuFrequency = 0;
uint64_t Stopwatch::ticksForOneUSleep = 0;

}
