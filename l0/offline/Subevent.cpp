
#include <new>
#include "Subevent.h"

namespace na62 {
namespace l0 {

Subevent::Subevent(const uint_fast16_t expectedPacketsNum, const uint_fast8_t sourceID) :
		expectedPacketsNum(expectedPacketsNum), sourceID(sourceID), eventFragments(
				new (std::nothrow) MEPFragment*[expectedPacketsNum]), fragmentCounter(
				0) {
}

Subevent::~Subevent() {
}

void Subevent::destroy() {
//TODO Delete the MEPFragment array
}
} /* namespace l0 */
} /* namespace na62 */
