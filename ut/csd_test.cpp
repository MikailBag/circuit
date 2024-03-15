#include "csd.h"


#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>


TEST_CASE("Canonical single digit encoder", "[csd]") {
    uint64_t x = GENERATE(0, 5, 15, 16, 19, 31, 179, 287, 345, 1234567, 415);

    CanonicalSignedDigitRepresentation csd = EncodeCsd(x);
    CHECK(csd.positive >= csd.negative);
    CHECK(csd.positive - csd.negative == x);
    CHECK((csd.positive & csd.negative) == 0);
    uint64_t nonzero = (csd.positive | csd.negative);
    CHECK(((nonzero>>1) & nonzero) == 0);
}
