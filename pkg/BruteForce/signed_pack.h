#pragma once

#include <cstdint>

namespace bf {
uint64_t PackSigned(int64_t x);
int64_t UnpackSigned(uint64_t x);
}