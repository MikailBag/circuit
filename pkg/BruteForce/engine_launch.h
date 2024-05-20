#pragma once

#include "bruteforce/beta_ext.h"
#include "bruteforce/params.h"
#include "bruteforce/topology.h"

#include "bitset.h"

namespace bf {
struct EngineParams {
    EvalConfig const& config;
    size_t inputCount;
    // TODO - remove
    size_t maxExplicitNodeCount;
    std::function<void()> progressListener;
    // required when engine is beta
    BetaExtensions const* betaExt;

    explicit EngineParams(EvalConfig const& c) : config(c) {} 
};


void InvokeEngine(EngineParams const& ep, LaunchConfig const& lp, std::vector<Topology> const& topologies, bs::BitSet<1>* out1, bs::BitSet<2>* out2);

bs::BitSet<1> PrepareBitset1(uint8_t maxBits);
bs::BitSet<2> PrepareBitset2(uint8_t maxBits);
}