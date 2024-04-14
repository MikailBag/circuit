#include "isomorphism_key.h"

#include "bruteforce/topology.h"

namespace bf::iso {
namespace {
uint64_t SimpleEval(Topology const& t, size_t inputCount) {
    std::array<uint64_t, kMaxExplicitNodeCount + kMaxInCount> vals;
    for (size_t i = 0; i < inputCount; i++) {
        vals[i] = i+1;
    }
    for (size_t i = 0; i < t.nodes.size(); i++) {
        vals[inputCount + i] = vals[t.nodes[i].links[0]] + vals[t.nodes[i].links[1]];
    }
    return vals[inputCount + t.nodes.size() - 1];
}
}

IsomorphismKey Get(Topology const& g, size_t inputCount) {
    std::array<size_t, kMaxExplicitNodeCount + 2> degrees;
    degrees.fill(0);
    for (size_t i = 0; i < g.nodes.size(); i++) {
        degrees[g.nodes[i].links[0]]++;
        degrees[g.nodes[i].links[1]]++;
    }
    IsomorphismKey ik;
    for (size_t i = 0; i < g.nodes.size(); i++) {
        ik.cnt[degrees[i]]++;
    }
    ik.output = SimpleEval(g, inputCount);
    return ik;
}
}