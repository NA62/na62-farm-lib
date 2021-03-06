
#include <new>
#include "Subevent.h"

namespace na62 {
namespace l0 {

Subevent::Subevent(const uint_fast16_t pexpectedPacketsNum) :
		expectedPacketsNum(pexpectedPacketsNum), sourceID(0), fragmentCounter(0), eventFragments(new (std::nothrow) MEPFragment*[pexpectedPacketsNum]) {

    for (uint index = 0; index < expectedPacketsNum; ++index) {
        eventFragments[index] = new MEPFragment();
    }
}

Subevent::~Subevent() {
}

void Subevent::destroy() {
//TODO Delete the MEPFragment array
}
} /* namespace l0 */
} /* namespace na62 */
