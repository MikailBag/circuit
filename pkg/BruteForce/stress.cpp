#include "bruteforce/stress.h"

#include "engine_launch.h"

namespace bf {

std::optional<Mismatch> Compare(FindOutputsParams const& left, FindOutputsParams const& right, std::vector<Topology> const& topologies) {
    EngineParams epLeft {left.config};
    EngineParams epRight {right.config};
    assert(left.inputCount == right.inputCount);
    epLeft.inputCount = left.inputCount;
    epRight.inputCount = right.inputCount;
    epLeft.maxExplicitNodeCount = left.maxExplicitNodeCount;
    epRight.maxExplicitNodeCount = right.maxExplicitNodeCount;
    assert(left.maxBits == right.maxBits);
    epLeft.maxBits = left.maxBits;
    epRight.maxBits = right.maxBits;
    epLeft.progressListener = left.progressListener;
    epRight.progressListener = right.progressListener;
    std::vector<Topology> tmp;
    if (left.inputCount == 1) {
        bs::BitSet<1> resultLeft = PrepareBitset1(left.maxBits);
        bs::BitSet<1> resultRight = PrepareBitset1(left.maxBits);
        for (size_t i = 0; i < topologies.size(); i++) {
            tmp.clear();
            tmp.push_back(topologies[i]);
            InvokeEngine(epLeft, tmp, &resultLeft, nullptr);
            InvokeEngine(epRight, tmp, &resultRight, nullptr);
            if (resultLeft != resultRight) {
                Mismatch m;
                m.topologyIndex = i;
                resultLeft.ApplyPointwiseXor(resultRight);
                auto [out] = resultLeft.FindOne();
                m.output = {out};
                m.rightResult = resultRight.At(out);
                m.leftResult = !m.rightResult;

                return {m};
            }
        }
        return {};
    } else if (left.inputCount == 2) {
        throw std::runtime_error("TODO");
    } else {
        std::abort();
    }
}
}