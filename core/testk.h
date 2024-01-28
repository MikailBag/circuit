#include <span>
#include <cstdint>

bool Test1(std::span<uint64_t> nums, std::span<uint64_t> succ, uint64_t target, uint8_t coefWidth);
bool Test2(std::span<uint64_t> nums, uint64_t target, uint8_t coefWidth);