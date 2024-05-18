#include "bruteforce/bruteforce.h"
#include "bruteforce/topology.h"

#include "bitset.h"
#include "engine_launch.h"
#include "signed_pack.h"

#include "isomorphism/filter.h"
#include "isomorphism/key.h"

#include "log/log.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>


namespace bf {
namespace {

static logger::Logger L = logger::Get("bf");

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

void ValidateEvalParams(EvalParams const& p) {
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

std::vector<Topology> detail::BruteforceEntrypoints::FindTopologies(FindTopologyParams const& p) {
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

std::vector<Topology> detail::BruteforceEntrypoints::FilterTopologies(FilterParams const& p, std::span<Topology const> topologies) {
    L().AttrU64("count", topologies.size()).Log("Filtering topologies");
    if (p.config.sharding.isRange) {
        size_t sz = topologies.size() / p.config.sharding.range.totalPartCount;
        size_t start = sz * p.config.sharding.range.rangeStart;
        size_t end = std::min(topologies.size(), sz * (p.config.sharding.range.rangeStart + p.config.sharding.range.rangeSize));
        topologies = topologies.subspan(start, end-start);
        L().AttrU64("remainingCount", topologies.size()).Log("Applied ranged sharding filter");
    }
    //L().AttrS("name", "unsafe").AttrU64("value", static_cast<uint64_t>(p.config.unsafe)).Log("Configuration");
    std::vector<std::pair<iso::IsomorphismKey, size_t>> keys;
    //keys.reserve(topologies.size());
    for (size_t i = 0; i < topologies.size(); i++) {
        keys.emplace_back(iso::Get(topologies[i], p.inputCount), i);
    }
    std::stable_sort(keys.begin(), keys.end());
    size_t groupCount = 0;
    for (size_t i = 0; i < topologies.size(); i++) {
        if (i == 0 || keys[i-1].first != keys[i].first) {
            groupCount++;
        }
    }
    L().AttrU64("count", groupCount).Log("Computed group keys");
    std::vector<iso::OrderedTopology> rawRes;
    size_t groupStart = 0;
    for (size_t i = 0; i < topologies.size(); i++) {
        if (i + 1 == topologies.size() || keys[i+1].first != keys[i].first) {
            Topology const* first = &topologies[groupStart];
            size_t cnt = i - groupStart + 1;
            iso::FilterTopologies({first, cnt}, p, rawRes);
            groupStart = i+1;
        }
    }
    L().AttrU64("count", rawRes.size()).Log("Sorting");
    std::stable_sort(rawRes.begin(), rawRes.end(), [](iso::OrderedTopology const& lhs, iso::OrderedTopology const& rhs){
        return lhs < rhs;
    });
    std::vector<Topology> res;
    for (auto& item : rawRes) {
        res.push_back(std::move(item.tp));
    }

    return res;
}

std::vector<int64_t> detail::BruteforceEntrypoints::EvalTopologies(EvalParams const& p, std::vector<Topology> const& topologies) {
    ValidateEvalParams(p);

    std::vector<int64_t> ans;
    EngineParams ep {p.config};
    ep.inputCount = p.inputCount;
    ep.maxExplicitNodeCount = p.maxExplicitNodeCount;
    ep.progressListener = p.progressListener;
    if (p.inputCount == 1) {
        bs::BitSet<1> out = PrepareBitset1(p.config.settings.maxBits);
        InvokeEngine(ep, p.launchConfig, topologies, &out, nullptr);
        for (size_t i = 0; i < out.size()[0]; ++i) {
            if (out.At(i)) {
                ans.push_back(UnpackSigned(static_cast<uint64_t>(i)));
            }
        }
    } else if (p.inputCount == 2) {
        bs::BitSet<2> out = PrepareBitset2(p.config.settings.maxBits);
        InvokeEngine(ep, p.launchConfig, topologies, nullptr, &out);
        for (size_t i = 0; i < out.size()[0]; i++) {
            for (size_t j = 0; j < out.size()[1]; j++) {
                if (out.At(i, j)) {
                    ans.push_back(UnpackSigned(static_cast<uint64_t>(i)));
                    ans.push_back(UnpackSigned(static_cast<uint64_t>(j)));
                }
            }
        }
    } else {
        std::abort();
    }
    return ans;
}
}