#pragma once

#include "params.h"
#include "topology.h"

#include <vector>

namespace bf {

std::vector<Topology> FindAllTopologies(FindTopologyParams const& p);

std::vector<Topology> FilterTopologies(FilterParams const& p, std::span<Topology const> topologies);

std::vector<uint64_t> FindAllOutputs(FindOutputsParams const& p, std::vector<Topology> const& topologies);

}