#pragma once

#include "arrayvec/arrayvec.h"


#include "core/topology.h"

namespace bf {
inline constexpr size_t kMaxInCount = 2;
inline constexpr size_t kMaxExplicitNodeCount = 6;

using Topology = arrayvec::ArrayVec<TopologyNode, kMaxExplicitNodeCount>;

}