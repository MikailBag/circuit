#pragma once

#include "arrayvec/arrayvec.h"


#include "core/topology.h"

namespace bf {
inline constexpr size_t kMaxInCount = 2;
inline constexpr size_t kMaxExplicitNodeCount = 6;

struct Topology {
    arrayvec::ArrayVec<TopologyNode, kMaxExplicitNodeCount> nodes;
};

void ValidateTopology(Topology const& t, size_t inputCount);
}