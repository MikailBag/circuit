#pragma once

#include "bruteforce/params.h"
#include "bruteforce/topology.h"

#include "bitset.h"

namespace bf {
struct EngineParams {
    OutputConfig const& config;
    size_t inputCount;
    uint8_t maxBits;
    // TODO - remove
    size_t maxExplicitNodeCount;
    std::function<void(ProgressEvent const&)> progressListener;

    explicit EngineParams(OutputConfig const& c) : config(c) {} 
};


void InvokeEngine(EngineParams const& ep, LaunchConfig const& lp, std::vector<Topology> const& topologies, bs::BitSet<1>* out1, bs::BitSet<2>* out2);

bs::BitSet<1> PrepareBitset1(uint8_t maxBits);
bs::BitSet<2> PrepareBitset2(uint8_t maxBits);
}