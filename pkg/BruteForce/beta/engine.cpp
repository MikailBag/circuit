#include "beta/engine.h"

#include "bruteforce/topology.h"

#include "log/log.h"

#include <span>
#include <stdexcept>


static logger::Logger L = logger::Get("bf.beta");

namespace bf::beta {
namespace {
void FindTopologyOutputs1(Topology const& t, uint8_t bits, bs::BitSet<1>& ans, std::span<uint64_t> buf, size_t pos) {
    //L().AttrU64("pos", pos).Log("Dbg");
    static constexpr size_t kInputCount = 1;
    size_t maxVal = 1 << static_cast<size_t>(bits);
    bool isSink = pos == kInputCount + t.nodes.size() - 1;
    auto push = [pos, isSink, maxVal, buf, t = std::cref(t), bits, ans = std::ref(ans)](uint64_t result){
        //L().AttrU64("pos", pos).AttrI64("val", result).Log("Dbg2");
        if (result > maxVal) {
            return;
        }
        if (isSink) {
            ans.get().Put(true, (result));
        } else {
            buf[pos] = result;
            FindTopologyOutputs1(t, bits, ans, buf, pos+1);
        }
    };
    if (pos < kInputCount) {
        for (uint64_t x = 1; x <= maxVal; x *= 2) {
            push(x);
        }
    } else {
        size_t lhsIdx = t.nodes[pos - kInputCount].links[0];
        size_t rhsIdx = t.nodes[pos - kInputCount].links[1];
        uint64_t lhs = buf[lhsIdx];
        uint64_t rhs0 = buf[rhsIdx];
        for (; lhs <= maxVal; lhs *= 2) {
            for (uint64_t rhs = rhs0; rhs <= maxVal; rhs *= 2) {
                push(lhs + rhs);
                if (lhs >= rhs) {
                    push(lhs - rhs);
                } else {
                    push(rhs - lhs);
                }
                if (rhs == 0) {
                    break;
                }
            }
            if (lhs == 0) {
                break;
            }
        }
    }
}

void FindTopologyOutputs2(Topology const& t, uint8_t bits, bs::BitSet<2>& ans, std::span<std::pair<int64_t, int64_t>> buf, size_t pos) {
    static constexpr size_t kInputCount = 2;
    size_t maxVal = 1 << static_cast<size_t>(bits);
    bool isSink = pos == kInputCount + t.nodes.size() - 1;
    auto push = [pos, isSink, maxVal, buf, t = std::cref(t), bits, ans = std::ref(ans)](int64_t resLhs, int64_t resRhs){
        if (static_cast<uint64_t>(std::max(std::abs(resLhs), std::abs(resRhs))) > maxVal) {
            return;
        }
        //L().AttrU64("pos", pos).AttrI64("lhs", resLhs).AttrI64("rhs", resRhs).Log("Pushing");
        if (isSink) {
            if (resRhs >= 0 && resLhs >= 0) {
                ans.get().Put(true, static_cast<uint64_t>(resLhs), static_cast<uint64_t>(resRhs));
            } else if (resLhs <= 0 && resRhs <= 0) {
                ans.get().Put(true, static_cast<uint64_t>(-resLhs), static_cast<uint64_t>(-resRhs));
            }
        } else {
            buf[pos] = {resLhs, resRhs};
            FindTopologyOutputs2(t, bits, ans, buf, pos+1);
            buf[pos] = {resLhs, -resRhs};
            FindTopologyOutputs2(t, bits, ans, buf, pos+1);
            buf[pos] = {-resLhs, resRhs};
            FindTopologyOutputs2(t, bits, ans, buf, pos+1);
            buf[pos] = {-resLhs, -resRhs};
            FindTopologyOutputs2(t, bits, ans, buf, pos+1);
        }
    };
    
    if (pos < kInputCount) {
        if (pos == 0) {
            for (uint64_t x = 1; x <= maxVal; x *= 2) {
                push(x, 0);
            }
        } else {
            for (uint64_t x = 1; x <= maxVal; x *= 2) {
                push(0, x);
            }
        }
        return;
    }
    size_t lhsIdx = t.nodes[pos - kInputCount].links[0];
    size_t rhsIdx = t.nodes[pos - kInputCount].links[1];
    std::pair<int64_t, int64_t> lhs = buf[lhsIdx];
    std::pair<int64_t, int64_t> rhs0 = buf[rhsIdx];
    uint64_t lhsMax = std::max(lhs.first, lhs.second);
    for (; lhsMax <= maxVal; (lhs.first *= 2), (lhs.second *= 2), (lhsMax *= 2)) {
        std::pair<int64_t, int64_t> rhs = rhs0;
        uint64_t rhsMax = std::max(std::abs(rhs0.first), std::abs(rhs0.second));
        for (; rhsMax <= maxVal; (rhs.first *= 2), (rhs.second *= 2), (rhsMax *= 2)) {
            push(lhs.first + rhs.first, lhs.second + rhs.second);
            push(lhs.first - rhs.first, lhs.second - rhs.second);
            if (rhsMax == 0) {
                break;
            }
        }
        if (lhsMax == 0) {
            break;
        }
    }
}
}

template<size_t N>
void FindAllOutputsBulk(size_t maxBits, std::function<void(ProgressEvent const&)> func, std::span<Topology const> topologies, bs::BitSet<N>& out) {
    L().AttrU64("cnt", topologies.size()).AttrS("engine", "beta").Log("Starting");
    size_t processed = 0;
    std::vector<std::conditional_t<N == 1, uint64_t, std::pair<int64_t, int64_t>>> buf;
    buf.resize(kMaxExplicitNodeCount + kMaxInCount);
    std::span bufView {buf.data(), buf.size()};
    for (Topology const& t : topologies) {
        ValidateTopology(t, N);
        if constexpr (N == 1) {
            FindTopologyOutputs1(t, maxBits, out, bufView, 0);
        } else if (N == 2) {
            FindTopologyOutputs2(t, maxBits, out, bufView, 0);
        } else {
            std::abort();
        }
        ++processed;
        ProgressEvent ev;
        ev.finished = processed;
        if (func) {
            func(ev);
        }
    }
}

template void FindAllOutputsBulk<1>(size_t, std::function<void(ProgressEvent const&)>, std::span<Topology const>, bs::BitSet<1>&);
template void FindAllOutputsBulk<2>(size_t, std::function<void(ProgressEvent const&)>, std::span<Topology const>, bs::BitSet<2>&);

}