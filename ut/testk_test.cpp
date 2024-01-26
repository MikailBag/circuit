#include "testk.h"
#include "catch2/catch_test_macros.hpp"

TEST_CASE("Test2", "[test2]") {
    std::vector<uint64_t> nums {3, 5, 11};
    REQUIRE(Test2(nums, 19) == true);
    REQUIRE(Test2(nums, 17) == true);
    REQUIRE(Test2(nums, 166) == false);
}
