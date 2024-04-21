#include "engine_launch.h"


#include "bitset.h"

#include "alpha/engine.h"
#include "beta/engine.h"

#include <cassert>

namespace bf {

void InvokeEngine(EngineParams const& p, std::vector<Topology> const& topologies, bs::BitSet<1>* out1, bs::BitSet<2>* out2) {
    size_t maxNum = static_cast<size_t>(1) << static_cast<size_t>(p.maxBits);
    if (p.inputCount == 1) {
        *out1 =  bs::BitSet{std::array{maxNum+1}};
        if (p.config.isAlpha) {
            alpha::FindAllOutputsBulk<1>(p.maxBits, p.maxExplicitNodeCount, p.progressListener, topologies, *out1);
        } else if (p.config.isBeta) {
            beta::FindAllOutputsBulk<1>(p.maxBits, p.progressListener, topologies, *out1);
        }
    } else if (p.inputCount == 2) {
        assert(out2 != nullptr);
        if (p.config.isAlpha) {
            alpha::FindAllOutputsBulk<2>(p.maxBits, p.maxExplicitNodeCount, p.progressListener, topologies, *out2);
        } else if (p.config.isBeta) {
            beta::FindAllOutputsBulk<2>(p.maxBits, p.progressListener, topologies, *out2);
        }
    } else {
        std::abort();
    }
}

namespace {
size_t GetSize(uint8_t maxBits) {
    return static_cast<size_t>(1) << static_cast<size_t>(maxBits);
}
}

bs::BitSet<1> PrepareBitset1(uint8_t maxBits) {
    size_t sz = GetSize(maxBits);
    return bs::BitSet{std::array{sz+1}};
}
bs::BitSet<2> PrepareBitset2(uint8_t maxBits) {
    size_t sz = GetSize(maxBits);
    return bs::BitSet{std::array{sz+1, sz+1}};
}
}