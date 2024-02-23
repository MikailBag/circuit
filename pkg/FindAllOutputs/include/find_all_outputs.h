#pragma once

#include "arrayvec/arrayvec.h"
#include "topology.h"



#include <vector>

namespace find_all_outputs {

inline constexpr size_t kInCount = 2;
inline constexpr size_t kMaxExplicitNodeCount = 6;

using Topology = arrayvec::ArrayVec<TopologyNode, kMaxExplicitNodeCount>;

struct FindTopologyParams {
    size_t explicitNodeCountLimit;
};

std::vector<Topology> FindAllTopologies(FindTopologyParams const& p);

std::vector<Topology> FilterTopologies(std::vector<Topology> const& topologies);

struct FindOutputsParams {
    uint8_t bits;
};

std::vector<uint64_t> FindAllOutputs(FindOutputsParams const& p, std::vector<Topology> const& topologies);
}