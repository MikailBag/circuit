#include "testk.h"

#include <algorithm>


bool Test1([[maybe_unused]] std::span<uint64_t> nums, std::span<uint64_t> succ, [[maybe_unused]] uint64_t target, [[maybe_unused]] uint8_t coefWidth) {
    return std::binary_search(succ.begin(), succ.end(), target);
}

bool Test2([[maybe_unused]] std::span<uint64_t> nums, [[maybe_unused]] uint64_t target, [[maybe_unused]] uint8_t coefWidth) {
    return false;
}
