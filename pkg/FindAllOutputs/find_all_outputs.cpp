#include "find_all_outputs.h"

#include <stdexcept>

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
}