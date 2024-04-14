#pragma once

#include "bruteforce/progress.h"
#include "bruteforce/topology.h"

#include "bitset.h"

#include <functional>

namespace bf::beta {
template<size_t N>
void FindAllOutputsBulk(size_t maxBits, std::function<void(ProgressEvent const&)> func, std::vector<Topology> const& topologies, bs::BitSet<N>& out);
}