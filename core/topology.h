#pragma once

#include <array>
#include <cstdint>

struct TopologyNode {
    std::array<uint8_t, 2> links;
};
