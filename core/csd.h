#pragma once
#include <cstdint>

struct CanonicalSignedDigitRepresentation {
    uint64_t positive;
    uint64_t negative;

    enum class Kind {
        POSITIVE,
        NEGATIVE,
        NONE
    };

    Kind Classify(uint8_t bit);

    uint8_t PopCount();
};

CanonicalSignedDigitRepresentation EncodeCsd(uint64_t x);