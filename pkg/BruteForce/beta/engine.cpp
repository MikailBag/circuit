#include "beta/engine.h"

#include "bruteforce/topology.h"

#include "log/log.h"

#include <span>
#include <stdexcept>
#include <utility>


static logger::Logger L = logger::Get("bf.beta");

namespace bf::beta {
namespace {
template<size_t N>
std::array<int64_t, N> GenerateInput(size_t pos) {
    std::array<int64_t, N> out;
    out.fill(0);
    out[pos] = 1;
    return out;
}

template<size_t N>
uint64_t AbsMax(std::array<int64_t, N> val) {
    uint64_t res = static_cast<uint64_t>(std::abs(val[0]));
    for (size_t i = 1; i < N; i++) {
        res = std::max(res, static_cast<uint64_t>(std::abs(val[i])));
    }
    return res;
}

template<size_t N>
std::array<int64_t, N> MulBy2(std::array<int64_t, N> val) {
    for (size_t i = 0; i < N; i++) {
        val[i] *= 2;
    }
    return val;
}

template<size_t N>
std::array<int64_t, N> AddArr(std::array<int64_t, N> lhs, std::array<int64_t, N> rhs) {
    for (size_t i = 0; i < N; i++) {
        lhs[i] += rhs[i];
    }
    return lhs;
}

template<size_t N>
std::array<int64_t, N> SubArr(std::array<int64_t, N> lhs, std::array<int64_t, N> rhs) {
    for (size_t i = 0; i < N; i++) {
        lhs[i] -= rhs[i];
    }
    return lhs;
}

template<size_t N>
std::array<int64_t, N> Normalize(std::array<int64_t, N> val) {
    bool needsNegation = false;
    for (size_t i = 0; i < N; i++) {
        if (val[i] < 0) {
            needsNegation = true;
        } else if (val[i] > 0) {
            break;
        }
    }
    if (!needsNegation) {
        return val;
    }

    for (size_t i = 0; i < N; i++) {
        val[i] = -val[i];
    }
    return val;
}

template<size_t N>
std::array<uint64_t, N> TryNormalizeForSink(std::array<int64_t, N> val, bool& ok) {
    bool allPos = true;
    bool allNeg = true;
    for (size_t i = 0; i < N; i++) {
        if (val[i] > 0) {
            allNeg = false;
        }
        if (val[i] < 0) {
            allPos = false;
        }
    }
    if (!allNeg && !allPos) {
        ok = false;
        return {};
    }
    std::array<uint64_t, N> out;
    if (allPos) {
        for (size_t i = 0; i < N; i++) {
            out[i] = static_cast<uint64_t>(val[i]);
        }
    } else {
        for (size_t i = 0; i < N; i++) {
            out[i] = static_cast<uint64_t>(-val[i]);
        }
    }
    ok = true;
    return out;
}

template<size_t N>
void FindTopologyOutputsImpl(Topology const& t, uint8_t bits, bs::BitSet<N>& ans, std::span<std::array<int64_t, N>> buf, size_t pos) {
    static_assert(N == 1 || N == 2, "Unsupported N");
    size_t maxVal = 1 << static_cast<size_t>(bits);
    bool isSink = pos == N + t.nodes.size() - 1;
    auto push = [pos, isSink, maxVal, buf, t = std::cref(t), bits, ans = std::ref(ans)](std::array<int64_t, N> res){
        if (AbsMax(res) > maxVal) {
            return;
        }
        if (isSink) {
            bool ok;
            std::array<uint64_t, N> idx = TryNormalizeForSink(res, ok);
            while (ok) {
                ans.get().PutArr(true, idx);
                res = MulBy2(res);
                ok = (AbsMax(res) <= maxVal);
                if (ok) {
                    idx = TryNormalizeForSink(res, ok);
                }
                if (AbsMax(res) == 0) {
                    break;
                }
            }
        } else {
            buf[pos] = res;
            FindTopologyOutputsImpl<N>(t, bits, ans, buf, pos+1);
        }
    };

    if (pos < N) {
        push(GenerateInput<N>(pos));
        /*
        if (pos == 0) {
            for (uint64_t x = 1; x <= maxVal; x *= 2) {
                push(x, 0);
            }
        } else {
            for (uint64_t x = 1; x <= maxVal; x *= 2) {
                push(0, x);
            }
        }*/
        return;
    }
    size_t lhsIdx = t.nodes[pos - N].links[0];
    size_t rhsIdx = t.nodes[pos - N].links[1];
    std::array<int64_t, N> lhsOrig = buf[lhsIdx];
    std::array<int64_t, N> rhsOrig = buf[rhsIdx];
    for ([[maybe_unused]] int shiftId: {0, 1}) {
        std::array<int64_t, N> lhs = lhsOrig;
        // lhs is shifted, rhs remains
        uint64_t lhsMax = AbsMax(lhs);
        // TODO: skip first iteration when shiftId is 0
        for (; lhsMax <= maxVal; (lhs = MulBy2(lhs)), (lhsMax *= 2)) {
            std::array<int64_t, N> res1 = AddArr(lhs, rhsOrig);
            std::array<int64_t, N> res2 = SubArr(lhs, rhsOrig);
            res1 = Normalize(res1);
            res2 = Normalize(res2);
            push(res1);
            push(res2);

        /*uint64_t rhsMax = std::max(std::abs(rhs0.first), std::abs(rhs0.second));
        for (; rhsMax <= maxVal; (rhs.first *= 2), (rhs.second *= 2), (rhsMax *= 2)) {
            push(lhs.first + rhs.first, lhs.second + rhs.second);
            push(lhs.first - rhs.first, lhs.second - rhs.second);
            push(rhs.first - lhs.first, rhs.second - lhs.second);

            if (rhsMax == 0) {
                break;
            }
        }*/
            if (lhsMax == 0) {
                break;
            }
        }
    
        std::swap(lhsOrig, rhsOrig);
    }
   
}

/*void FindTopologyOutputs1(Topology const& t, uint8_t bits, bs::BitSet<1>& ans, std::span<uint64_t> buf, size_t pos) {
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
        if (isSink) {
            if (resRhs >= 0 && resLhs >= 0) {
                ans.get().Put(true, static_cast<uint64_t>(resLhs), static_cast<uint64_t>(resRhs));
            } else if (resLhs <= 0 && resRhs <= 0) {
                ans.get().Put(true, static_cast<uint64_t>(-resLhs), static_cast<uint64_t>(-resRhs));
            }
        } else {
            buf[pos] = {resLhs, resRhs};
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
    uint64_t lhsMax = std::max(std::abs(lhs.first), std::abs(lhs.second));
    for (; lhsMax <= maxVal; (lhs.first *= 2), (lhs.second *= 2), (lhsMax *= 2)) {
        std::pair<int64_t, int64_t> rhs = rhs0;
        uint64_t rhsMax = std::max(std::abs(rhs0.first), std::abs(rhs0.second));
        for (; rhsMax <= maxVal; (rhs.first *= 2), (rhs.second *= 2), (rhsMax *= 2)) {
            push(lhs.first + rhs.first, lhs.second + rhs.second);
            push(lhs.first - rhs.first, lhs.second - rhs.second);
            push(rhs.first - lhs.first, rhs.second - lhs.second);

            if (rhsMax == 0) {
                break;
            }
        }
        if (lhsMax == 0) {
            break;
        }
    }
}
*/
}

template<size_t N>
void FindAllOutputsBulk(size_t maxBits, std::function<void()> func, std::span<Topology const> topologies, bs::BitSet<N>& out, OutputBetaConfig const& conf) {
    L().AttrU64("cnt", topologies.size()).AttrS("engine", "beta").Log("Starting");
    std::vector<std::array<int64_t, N>> buf;
    buf.resize(kMaxExplicitNodeCount + kMaxInCount);
    std::span bufView {buf.data(), buf.size()};
    for (Topology const& t : topologies) {
        if (!conf.skipValidation) {
            ValidateTopology(t, N);
        }
        if (!conf.forceDummy) {

            /*if constexpr (N == 1) {
                FindTopologyOutputs1(t, maxBits, out, bufView, 0);
            } else if (N == 2) {
                FindTopologyOutputs2(t, maxBits, out, bufView, 0);
            } else {
                std::abort();
            }*/
            FindTopologyOutputsImpl<N>(t, maxBits, out, bufView, 0);
        }
        if (func) {
            func();
        }
    }
}

template void FindAllOutputsBulk<1>(size_t, std::function<void()>, std::span<Topology const>, bs::BitSet<1>&, OutputBetaConfig const& conf);
template void FindAllOutputsBulk<2>(size_t, std::function<void()>, std::span<Topology const>, bs::BitSet<2>&, OutputBetaConfig const& conf);

}