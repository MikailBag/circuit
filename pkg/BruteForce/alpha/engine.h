#pragma once

#include "bruteforce/progress.h"
#include "bruteforce/topology.h"

#include "bitset.h"

#include <functional>

namespace bf::alpha {    
template<size_t N>
void FindAllOutputsBulk(size_t maxBits, size_t explNodeCount, std::function<void(ProgressEvent const&)> func, std::vector<Topology> const& topologies, bs::BitSet<N>& out);
}