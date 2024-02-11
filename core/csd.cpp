#include "csd.h"

#include <algorithm>

CanonicalSignedDigitRepresentation EncodeCsd(uint64_t x) {
    // https://en.wikipedia.org/wiki/Non-adjacent_form, "Converting to NAF"
    CanonicalSignedDigitRepresentation ans;
    uint64_t xh = x >> 1;
    uint64_t x3 = x + xh;
    uint64_t c = xh ^ x3;
    ans.positive = x3 & c;
    ans.negative = xh & c;
    return ans;
}

uint8_t CanonicalSignedDigitRepresentation::PopCount() {
    return std::popcount(negative) + std::popcount(positive);
}

CanonicalSignedDigitRepresentation::Kind CanonicalSignedDigitRepresentation::Classify(uint8_t bit) {
    uint64_t mask = 1;
    mask <<= bit;
    if (positive & mask) {
        return Kind::POSITIVE;
    }
    if (negative & mask) {
        return Kind::NEGATIVE;
    }
    return Kind::NONE;
}