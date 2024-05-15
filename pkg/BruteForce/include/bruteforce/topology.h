#pragma once

#include "bruteforce/limits.h"

#include "arrayvec/arrayvec.h"


#include "core/topology.h"

namespace bf {

struct Topology {
    arrayvec::ArrayVec<TopologyNode, kMaxExplicitNodeCount> nodes;

    friend auto operator<=>(Topology const& lhs, Topology const& rhs) = default;
};

void ValidateTopology(Topology const& t, size_t inputCount);
}