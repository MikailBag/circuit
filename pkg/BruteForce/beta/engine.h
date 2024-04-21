#pragma once

#include "bruteforce/progress.h"
#include "bruteforce/topology.h"

#include "bitset.h"

#include <functional>
#include <span>

namespace bf::beta {
template<size_t N>
void FindAllOutputsBulk(size_t maxBits, std::function<void(ProgressEvent const&)> func, std::span<Topology const> topologies, bs::BitSet<N>& out);
}