
#include <new>
#include "Subevent.h"

extern "C" na62::l0::Subevent* create_subevent(const uint_fast16_t pexpectedPacketsNum) {
    return new na62::l0::Subevent(pexpectedPacketsNum);
}

extern "C" void destroy_subevent(na62::l0::Subevent* object) {
    delete object;
}


namespace na62 {
namespace l0 {


Subevent::Subevent(const uint_fast16_t pexpectedPacketsNum):
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
