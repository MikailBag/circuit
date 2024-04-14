#include "engine.h"

#include "eval_topology.h"

#include "bitset.h"

#include "log/log.h"




static Logger L = GetLogger("bf.alpha");

namespace bf::alpha {
template<size_t N>
void FindAllOutputsBulk(size_t maxBits, size_t explNodeCount, std::function<void(ProgressEvent const&)> func, std::vector<Topology> const& topologies, bs::BitSet<N>& out) {
    L().AttrU64("topologies", topologies.size()).AttrU64("inputCount", N).AttrU64("bits", maxBits).Log("Starting Alpha engine");
    size_t maxNum = static_cast<size_t>(1) << static_cast<size_t>(maxBits);
    std::vector<bs::BitSet<N>> bufs;
    bufs.reserve(explNodeCount + N);
    for (size_t i = 0; i < explNodeCount + N; ++i) {
        if constexpr (N == 1) {
            bufs.emplace_back(std::array<size_t, 1>{maxNum+1});
        } else if (N == 2) {
            bufs.emplace_back(std::array<size_t, 2>{maxNum+1, maxNum + 1});
        } else {
            std::abort();
        }
    }
    if constexpr (N == 1) {
        bufs[0].Put(true, static_cast<size_t>(1));
        PushShifts(bufs[0]);
    } else if (N == 2) {
        bufs[0].Put(true, static_cast<size_t>(1), static_cast<size_t>(0));
        PushShifts(bufs[0]);
        bufs[1].Put(true, static_cast<size_t>(0), static_cast<size_t>(1));
        PushShifts(bufs[1]);
    } else {
        std::abort();
    }
    size_t processed = 0;
    for (Topology const& t : topologies) {
        if (t.nodes.size() > explNodeCount) {
            throw std::invalid_argument("topology is bigger than maxExplicitNodeCount");
        }
        FindTopologyOutputs(t, N, maxBits, out, bufs);
        ++processed;
        ProgressEvent ev;
        ev.finished = processed;
        if (func) {
            func(ev);
        }
    }

}


template void FindAllOutputsBulk<1>(size_t, size_t, std::function<void(ProgressEvent const&)>, std::vector<Topology> const&, bs::BitSet<1>&);
template void FindAllOutputsBulk<2>(size_t, size_t, std::function<void(ProgressEvent const&)>, std::vector<Topology> const&, bs::BitSet<2>&);
}