#include "find_all_outputs.h"

#include <stdexcept>
#include <string>


namespace find_all_outputs {
namespace {
void GoStep([[maybe_unused]] FindTopologyParams const& p, Topology const& g, std::vector<Topology>& out) {
    for (size_t i = 0; i < g.size() + kInCount; ++i) {
        for (size_t j = i; j < g.size() + kInCount; ++j) {
            TopologyNode n;
            n.links[0] = i;
            n.links[1] = j;
            Topology g2{g};
            g2.push_back(n);
            out.push_back(std::move(g2));
        }
    }
}

void Go(FindTopologyParams const& p, std::vector<Topology> const& curLayer, std::vector<Topology>& nextLayer) {
    for (Topology const& g : curLayer) {
        GoStep(p, g, nextLayer);
    }
}

void PushShifts(std::vector<bool>& v) {
    size_t n = v.size();
    for (size_t i = 0; 2*i < n; i++) {
        if (v[i]) {
            v[2*i] = true;
        }
    }
}

void Combine(std::vector<bool> const& lhs, std::vector<bool> const& rhs, std::vector<bool>& out) {
    size_t n = lhs.size();
    assert(n == rhs.size());
    assert(n == out.size());
    out.assign(n, false);
    for (size_t i = 0; i < n; i++) {
        if (!lhs[i]) {
            continue;
        }
        for (size_t j = 0; i + j < n; j++) {
            if (!rhs[j]) {
                continue;
            }
            out[i+j] = true;
        }
    }
}

void FindAllOutputsOfTopology([[maybe_unused]] Topology const& t, [[maybe_unused]] uint8_t bits, [[maybe_unused]] std::vector<bool>& ans, std::vector<std::vector<bool>> bufs) {
    size_t n = 1 << static_cast<size_t>(bits);
    for (size_t i = 0; i < t.size(); i++) {
        bufs[i].assign(n, false);
    }
    for (size_t i = 0; i < t.size(); i++) {
        size_t lhsIdx = t[i].links[0];
        size_t rhsIdx = t[i].links[1];
        Combine(bufs[lhsIdx], bufs[rhsIdx], bufs[i+2]);
        PushShifts(bufs[i+2]);
    }
}
}

std::vector<Topology> FindAllTopologies(FindTopologyParams const& p) {
    std::vector<Topology> out;
    out.push_back(Topology{});
    std::vector<Topology> tmp;
    for (size_t i = 0; i < p.explicitNodeCountLimit; ++i) {
        Go(p, out, tmp);
        std::swap(out, tmp);
        tmp.clear();
    }
    return out;
}

std::vector<Topology> FilterTopologies(std::vector<Topology> const& topologies) {
    return topologies;
}

std::vector<uint64_t> FindAllOutputs([[maybe_unused]] FindOutputsParams const& p, [[maybe_unused]] std::vector<Topology> const& topologies) {
    std::vector<bool> outs;
    size_t maxNum = static_cast<size_t>(1) << static_cast<size_t>(p.bits);
    outs.resize(maxNum);
    std::vector<std::vector<bool>> bufs;
    bufs.resize(kMaxExplicitNodeCount + 2);
    bufs[0].assign(maxNum, false);
    bufs[0][0] = true;
    PushShifts(bufs[0]);
    bufs[1] = bufs[0];
    for (Topology const& t : topologies) {
        FindAllOutputsOfTopology(t, p.bits, outs, bufs);
    }
    std::vector<uint64_t> ans;
    for (size_t i = 0; i < outs.size(); ++i) {
        if (outs[i]) {
            ans.push_back(static_cast<uint64_t>(i));
        }
    }
    throw std::runtime_error("TODO");
}
}