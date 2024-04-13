#pragma once

#include "bruteforce/progress.h"
#include "bruteforce/topology.h"

#include <functional>

namespace bf::alpha {    
template<size_t N>
std::vector<uint64_t> FindAllOutputsBulk(size_t maxBits, size_t explNodeCount, std::function<void(ProgressEvent const&)> func, std::vector<Topology> const& topologies);
}