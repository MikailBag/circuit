#include "min_adder_depth.h"

#include "csd.h"

#include <bit>
#include <cassert>

size_t MinAdderDepth(uint64_t x) {
    assert(x != 0);
    CanonicalSignedDigitRepresentation csd = EncodeCsd(x);
    size_t nz = csd.popcnt();// std::popcount(csd.negative) + std::popcount(csd.positive);
    size_t ans = 0;
    size_t acc = 1;
    while (acc < nz) {
        acc *= 2;
        ++ans;
    }
    return ans;
}
