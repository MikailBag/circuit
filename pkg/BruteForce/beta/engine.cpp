#include "beta/engine.h"

#include "bruteforce/topology.h"

#include "signed_pack.h"

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
            break;
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

/**/

template<size_t N>
std::array<uint64_t, N> ToIndex(std::array<int64_t, N> val) {
    std::array<uint64_t, N> res;
    for (size_t i = 0; i < N; i++) {
        res[i] = PackSigned(val[i]);
    }
    return res;
}

template<size_t N>
struct State {
    uint8_t bits;
    bool enableSecondOutputFilter = false;
    std::array<int64_t, N> expectedSecondOutput;

};

template<size_t N>
void FindTopologyOutputsImpl(State<N> const& s, Topology const& t, bs::BitSet<N>& ans, std::span<std::array<int64_t, N>> buf, size_t pos) {
    static_assert(N == 1 || N == 2, "Unsupported N");
    size_t maxVal = 1 << static_cast<size_t>(s.bits);
    bool isSink = pos == N + t.nodes.size() - 1;
    bool isPotentialSecondOutput = (t.nodes.size() >= 2) && (pos == N + t.nodes.size() - 2);
    auto push = [pos, isSink, isPotentialSecondOutput, maxVal, buf, t = std::cref(t), s = &s, ans = std::ref(ans)](std::array<int64_t, N> res){
        if (AbsMax(res) > maxVal) {
            return;
        }
        if (isPotentialSecondOutput && s->enableSecondOutputFilter) {
            
        }
        if (isSink) {
            if (s->enableSecondOutputFilter) {
                std::array<int64_t, N> prevRes = buf[pos-1];
                if (prevRes != s->expectedSecondOutput) {
                    if (res != s->expectedSecondOutput) {
                        return;
                    }
                    res = prevRes;
                }
            }
            while (true) {
                ans.get().PutArr(true, ToIndex(res));
                res = MulBy2(res);
                uint64_t mx = AbsMax(res);
                if (mx == 0 || mx > maxVal) {
                    break;
                }
            }
        } else {
            buf[pos] = res;
            FindTopologyOutputsImpl<N>(*s, t, ans, buf, pos+1);
        }
    };

    if (pos < N) {
        push(GenerateInput<N>(pos));
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
            if (lhsMax == 0) {
                break;
            }
        }
    
        std::swap(lhsOrig, rhsOrig);
    }
   
}
}

template<size_t N>
void FindAllOutputsBulk(EvalConfig::Settings const& settings, std::function<void()> func, std::span<Topology const> topologies, bs::BitSet<N>& out, EvalBetaConfig const& conf) {
    L().AttrU64("cnt", topologies.size()).AttrS("engine", "beta").Log("Starting");
    std::vector<std::array<int64_t, N>> buf;
    buf.resize(kMaxExplicitNodeCount + kMaxInCount);
    std::span bufView {buf.data(), buf.size()};
    for (Topology const& t : topologies) {
        if (!conf.skipValidation) {
            ValidateTopology(t, N);
        }
        if (!conf.forceDummy) {
            State<N> s;
            if (settings.secondOutput.isEnabled) {
                s.enableSecondOutputFilter = true;
                if constexpr (N == 1) {
                    s.expectedSecondOutput = {settings.secondOutput.enabled.x};
                } else if (N == 2) {
                    s.expectedSecondOutput = {settings.secondOutput.enabled.x, settings.secondOutput.enabled.y};
                } else {
                    assert(false);
                }
            }
            s.bits = settings.maxBits;
            
            FindTopologyOutputsImpl<N>(s, t, out, bufView, 0);
        }
        if (func) {
            func();
        }
    }
}

template void FindAllOutputsBulk<1>(EvalConfig::Settings const&, std::function<void()>, std::span<Topology const>, bs::BitSet<1>&, EvalBetaConfig const& conf);
template void FindAllOutputsBulk<2>(EvalConfig::Settings const&, std::function<void()>, std::span<Topology const>, bs::BitSet<2>&, EvalBetaConfig const& conf);

}