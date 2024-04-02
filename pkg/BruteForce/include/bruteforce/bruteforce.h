#pragma once

#include "topology.h"

#include <vector>

namespace bruteforce {


struct FindTopologyParams {
    size_t inputCount = 0;
    size_t explicitNodeCountLimit = 0;
};

std::vector<Topology> FindAllTopologies(FindTopologyParams const& p);

struct FilterParams {
    size_t inputCount = 0;
    bool unsafe = false;
};

std::vector<Topology> FilterTopologies(FilterParams const& p, std::vector<Topology> const& topologies);

struct FindOutputsParams {
    uint8_t maxBits = 0;
    size_t maxExplicitNodeCount = 0;
    size_t inputCount = 0;
};

std::vector<uint64_t> FindAllOutputs(FindOutputsParams const& p, std::vector<Topology> const& topologies);
}