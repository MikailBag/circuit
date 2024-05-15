#include "isomorphism/filter.h"

#include "bruteforce/topology.h"

#include "log/log.h"

static logger::Logger L = logger::Get("bf.iso.filter");

namespace bf::iso {

namespace {
bool HasSingleSink(Topology const& t, size_t inputCount) {
    std::array<bool, kMaxExplicitNodeCount+kMaxInCount> hasEgress;
    hasEgress.fill(false);
    for (size_t i = 0; i < t.nodes.size(); i++) {
        hasEgress[t.nodes[i].links[0]] = true;
        hasEgress[t.nodes[i].links[1]] = true;
    }
    size_t sinks = 0;
    for (size_t i = 0; i < t.nodes.size(); i++) {
        if (!hasEgress[i+inputCount]) {
            sinks++;
        }
    }
    return sinks < 2;
}

bool CheckEquivalent(Topology const& lhs, Topology const& rhs, size_t inputCount) {
    size_t sz = lhs.nodes.size();
    std::array<size_t, kMaxExplicitNodeCount> perm;
    for (size_t i = 0; i < sz; i++) {
        perm[i] = i;
    }
    do {
        bool ok = true;
        for (size_t i = 0; i < sz; i++) {
            size_t u = lhs.nodes[i].links[0];
            if (u < inputCount) {
                if (rhs.nodes[i].links[0] != u) {
                    ok = false;
                    break;
                }
                u -= inputCount;
            }
            size_t v = lhs.nodes[i].links[1];
            if (v < inputCount) {
                if (rhs.nodes[i].links[1] != v) {
                    ok = false;
                    break;
                }
            }
            if (perm[u] != rhs.nodes[perm[i]].links[0]) {
                ok = false;
                break;
            }
            if (perm[v] != rhs.nodes[perm[i]].links[1]) {
                ok = false;
                break;
            }
        }
        if (ok) {
            return true;
        }
    } while (std::next_permutation(perm.begin(), perm.begin() + sz));
    return false;
}
}

void FilterTopologies(std::span<Topology const> topologies, FilterParams const& p, std::vector<OrderedTopology>& output) {
    Order ord;
    size_t sz = output.size();
    for (Topology t : topologies) {
        if (!HasSingleSink(t, p.inputCount)) {
            continue;
        }
        if (p.config.enableIsomorphismCheck) {
            for (size_t i = sz; i < output.size(); i++) {
                if (CheckEquivalent(output[i].tp, t, p.inputCount)) {
                    continue;
                }
            }
        }
        OrderedTopology ot;
        ot.tp = Topology{t};
        ot.ord = std::exchange(ord, Order::kNormal);
        output.push_back(std::move(ot));
    }
}

}