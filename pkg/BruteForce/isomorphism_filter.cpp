#include "isomorphism_filter.h"

#include "bruteforce/topology.h"

#include "log/log.h"

static Logger L = GetLogger("bf.iso.filter");

namespace bruteforce::iso {

namespace {
bool HasSingleSink(Topology const& t, size_t inputCount) {
    std::array<bool, kMaxExplicitNodeCount+kMaxInCount> used;
    used.fill(false);
    for (size_t i = 0; i < t.size(); i++) {
        used[t[i].links[0]] = true;
        used[t[i].links[1]] = true;
    }
    size_t unused = 0;
    for (size_t i = 0; i < t.size(); i++) {
        if (!used[i+inputCount]) {
            unused++;
        }
    }
    return unused < 2;
}
}

void FilterTopologies(std::span<Topology const> topologies, FilterParams const& p, std::vector<Topology>& output) {
    for (Topology t : topologies) {
        if (!HasSingleSink(t, p.inputCount)) {
            continue;
        }
        output.push_back(t);
        if (p.unsafe) {
            break;
        }
    }
}

}