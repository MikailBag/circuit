#pragma once

#include "bruteforce/topology.h"

#include <array>

namespace bf::iso {
struct IsomorphismKey {
    std::array<size_t, kMaxExplicitNodeCount * 2> cnt;
    uint64_t output = 0;
    IsomorphismKey() {
        cnt.fill(0);
    }

    friend auto operator<=>(IsomorphismKey const& lhs, IsomorphismKey const& rhs) = default;
};

IsomorphismKey Get(Topology const& g, size_t inputCount);
}