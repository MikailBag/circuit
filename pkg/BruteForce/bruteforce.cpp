#include "bruteforce/bruteforce.h"
#include "bruteforce/topology.h"

#include "bitset.h"
#include "isomorphism_filter.h"
#include "isomorphism_key.h"

#include "alpha/engine.h"
#include "beta/engine.h"

#include "log/log.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>



namespace bf {
namespace {

static Logger L = GetLogger("bf");

void GoStep(FindTopologyParams const& p, Topology const& g, std::vector<Topology>& out) {
    for (size_t i = 0; i < g.nodes.size() + p.inputCount; ++i) {
        for (size_t j = i; j < g.nodes.size() + p.inputCount; ++j) {
            TopologyNode n;
            n.links[0] = i;
            n.links[1] = j;
            Topology g2{g};
            g2.nodes.push_back(n);
            out.push_back(std::move(g2));
        }
    }
}

void Go(FindTopologyParams const& p, std::vector<Topology> const& curLayer, std::vector<Topology>& nextLayer) {
    for (Topology const& g : curLayer) {
        GoStep(p, g, nextLayer);
    }
}

void ValidateFindOutputParams(FindOutputsParams const& p) {
    if (p.maxExplicitNodeCount > kMaxExplicitNodeCount) {
        throw std::invalid_argument("maxExplicitNodeCount is too big");
    }
    if (p.inputCount != 1 && p.inputCount != 2) {
        throw std::invalid_argument("supported input count are 1 and 2");
    }
}


void ValidateFindTopologyParams(FindTopologyParams const& p) {
    if (p.inputCount != 1 && p.inputCount != 2) {
        throw std::invalid_argument("supported input count are 1 and 2");
    }
}
}

std::vector<Topology> FindAllTopologies(FindTopologyParams const& p) {
    ValidateFindTopologyParams(p);
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

std::vector<Topology> FilterTopologies(FilterParams const& p, std::vector<Topology> const& topologies) {
    L().AttrU64("count", topologies.size()).Log("Filtering topologies");
    L().AttrS("name", "unsafe").AttrU64("value", static_cast<uint64_t>(p.config.unsafe)).Log("Configuration");
    std::vector<std::pair<iso::IsomorphismKey, size_t>> keys;
    //keys.reserve(topologies.size());
    for (size_t i = 0; i < topologies.size(); i++) {
        keys.emplace_back(iso::Get(topologies[i], p.inputCount), i);
    }
    std::sort(keys.begin(), keys.end());
    size_t groupCount = 0;
    for (size_t i = 0; i < topologies.size(); i++) {
        if (i == 0 || keys[i-1].first != keys[i].first) {
            groupCount++;
        }
    }
    L().AttrU64("count", groupCount).Log("Computed group keys");
    std::vector<Topology> res;
    size_t groupStart = 0;
    for (size_t i = 0; i < topologies.size(); i++) {
        if (i + 1 == topologies.size() || keys[i+1].first != keys[i].first) {
            Topology const* first = &topologies[groupStart];
            size_t cnt = i - groupStart + 1;
            iso::FilterTopologies({first, cnt}, p, res);
            groupStart = i+1;
        }
    }
    return res;
}

std::vector<uint64_t> FindAllOutputs(FindOutputsParams const& p, std::vector<Topology> const& topologies) {
    ValidateFindOutputParams(p);
    size_t maxNum = static_cast<size_t>(1) << static_cast<size_t>(p.maxBits);
    std::vector<uint64_t> ans;
    if (p.inputCount == 1) {
        bs::BitSet<1> out =  bs::BitSet{std::array{maxNum+1}};
        if (p.config.isAlpha) {
            alpha::FindAllOutputsBulk<1>(p.maxBits, p.maxExplicitNodeCount, p.progressListener, topologies, out);
        } else if (p.config.isBeta) {
            beta::FindAllOutputsBulk<1>(p.maxBits, p.progressListener, topologies, out);
        }
        for (size_t i = 0; i < out.size()[0]; ++i) {
            if (out.At(i)) {
                ans.push_back(static_cast<uint64_t>(i));
            }
        }
    } else if (p.inputCount == 2) {
        bs::BitSet<2> out = bs::BitSet{std::array{maxNum+1, maxNum+1}};
        if (p.config.isAlpha) {
            alpha::FindAllOutputsBulk<2>(p.maxBits, p.maxExplicitNodeCount, p.progressListener, topologies, out);
        } else if (p.config.isBeta) {
            beta::FindAllOutputsBulk<2>(p.maxBits, p.progressListener, topologies, out);
        }
        for (size_t i = 0; i < out.size()[0]; i++) {
            for (size_t j = 0; j < out.size()[1]; j++) {
                if (out.At(i, j)) {
                    ans.push_back(static_cast<uint64_t>(i));
                    ans.push_back(static_cast<uint64_t>(j));
                }
            }
        }
    } else {
        std::abort();
    }
    return ans;
}
}