#pragma once

#include "config.h"
#include "progress.h"
#include "topology.h"


#include <functional>
#include <vector>

namespace bf {


struct FindTopologyParams {
    size_t inputCount = 0;
    size_t explicitNodeCountLimit = 0;
};

std::vector<Topology> FindAllTopologies(FindTopologyParams const& p);

struct FilterParams {
    size_t inputCount = 0;
    FilterConfig const& config;
};

std::vector<Topology> FilterTopologies(FilterParams const& p, std::vector<Topology> const& topologies);


struct FindOutputsParams {
    OutputConfig const& config;
    uint8_t maxBits = 0;
    size_t maxExplicitNodeCount = 0;
    size_t inputCount = 0;
    std::function<void(ProgressEvent const&)> progressListener;
};

std::vector<uint64_t> FindAllOutputs(FindOutputsParams const& p, std::vector<Topology> const& topologies);
}