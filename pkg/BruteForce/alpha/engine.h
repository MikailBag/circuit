#pragma once

#include "bruteforce/topology.h"

#include "bitset.h"

#include <functional>
#include <span>

namespace bf::alpha {    
template<size_t N>
void FindAllOutputsBulk(size_t maxBits, size_t explNodeCount, std::function<void()> func, std::span<Topology const> topologies, bs::BitSet<N>& out);
}