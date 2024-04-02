#pragma once
#include "bruteforce/topology.h"

#include "bitset.h"

#include <vector>
namespace bruteforce {

template<size_t N>
void PushShifts(bs::BitSet<N>& v);

template<size_t N>
void FindTopologyOutputs(Topology const& t, size_t inputs, uint8_t bits, bs::BitSet<N>& ans, std::vector<bs::BitSet<N>> bufs);
}