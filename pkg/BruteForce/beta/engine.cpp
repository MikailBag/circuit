#include "beta/engine.h"

#include "beta/restore_graph.h"
#include "beta/well_known.h"

#include "bruteforce/topology.h"

#include "signed_pack.h"

#include "log/log.h"

#include <iostream>
#include <format>
#include <functional>
#include <span>
#include <stdexcept>
#include <unordered_set>
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
    bool wellKnown = false;
    std::array<int64_t, N> expectedSecondOutput;

    bool enablePrintTopology = false;
    std::array<int64_t, N> expectedOutput;
    std::function<void(Graph)> topologyPrinter;

};

template<class Out, size_t N>
concept OutputCallback = 
    // std::is_same_v<typeof(Out::SupportsSecondOutput), bool> &&
    requires(Out out, std::array<int64_t, N> res, ssize_t wkIndex) {
    { out.RecordOutput(res) } -> std::same_as<void>;
    { out.RecordSecondOutput(res, wkIndex) } -> std::same_as<void>;
};

template<size_t N, OutputCallback<N> Out>
void FindTopologyOutputsImpl(State<N> const& s, Topology const& t, Out& out, std::span<std::array<int64_t, N>> buf, size_t pos) {
    static_assert(N == 1 || N == 2, "Unsupported N");
    size_t maxVal = 1 << static_cast<size_t>(s.bits);
    bool isSink = pos == N + t.nodes.size() - 1;
    bool isPotentialSecondOutput = (t.nodes.size() >= 2) && (pos == N + t.nodes.size() - 2);
    auto push = [pos, isSink, isPotentialSecondOutput, maxVal, buf, t = std::cref(t), s = &s, out = &out](std::array<int64_t, N> res){
        if (AbsMax(res) > maxVal) {
            return;
        }
        if (isSink) {
            ssize_t wellKnownIndex = SSIZE_MAX;
            if (Out::SupportsSecondOutput && s->enableSecondOutputFilter) {
                std::array<int64_t, N> prevRes = buf[pos-1];
                if (s->wellKnown) {
                    assert(N == 2);
                    wellKnownIndex = IsWellKnown(prevRes[0], prevRes[1]);
                    if (wellKnownIndex < 0) {
                        wellKnownIndex = IsWellKnown(res[0], res[1]);
                        if (wellKnownIndex < 0) {
                            return;
                        }
                        res = prevRes;
                    }
                } else {
                    if (prevRes != s->expectedSecondOutput) {
                        if (res != s->expectedSecondOutput) {
                            return;
                        }
                        res = prevRes;
                    }
                }
            }
            while (true) {
                if (s->enablePrintTopology && res == s->expectedOutput) {
                    buf[pos] = res;
                    s->topologyPrinter(Restore(t, buf.subspan(0, t.get().nodes.size() + N)));
                }
                if (Out::SupportsSecondOutput && s->enableSecondOutputFilter && s->wellKnown) {
                    out->RecordSecondOutput(res, wellKnownIndex);
                } else {
                    out->RecordOutput(res);
                }
                res = MulBy2(res);
                uint64_t mx = AbsMax(res);
                if (mx == 0 || mx > maxVal) {
                    break;
                }
            }
        } else {
            buf[pos] = res;
            FindTopologyOutputsImpl<N, Out>(*s, t, *out, buf, pos+1);
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

template<size_t N>
class SimpleOutputCallback {
public:
    explicit SimpleOutputCallback(bs::BitSet<N>& out) : mOut(out) {
    }

    void RecordOutput(std::array<int64_t, N> res) {
        mOut.PutArr(true, ToIndex(res));
    }

    inline constexpr static bool SupportsSecondOutput = false;

    void RecordSecondOutput([[maybe_unused]] std::array<int64_t, N> res, [[maybe_unused]] ssize_t idx) {
        assert(false);
    }
private:
    bs::BitSet<N>& mOut;
};

template<size_t N>
struct ArrayHasher {
public:
    size_t operator()(std::array<int64_t, N> arr) const noexcept {
        if (N == 1) {
            return std::bit_cast<size_t>(arr[0]);
        }
        if (N == 2) {
            size_t x = std::bit_cast<size_t>(arr[0]);
            size_t y = std::bit_cast<size_t>(arr[1]);
            return (x << 1) | y;
        }
        assert(N == 1 || N == 2);
    }

};

template<size_t N>
class ExpensiveOutputCallback {
public:
    explicit ExpensiveOutputCallback(size_t wkCount) : mBufs(wkCount) {}
    void RecordOutput([[maybe_unused]] std::array<int64_t, N> res) {
        assert(false);
    }

    inline constexpr static bool SupportsSecondOutput = true;

    void RecordSecondOutput(std::array<int64_t, N> res, ssize_t idx) {
        mBufs[idx].insert(res);
    }
private:
    std::vector<std::unordered_set<std::array<int64_t, N>, ArrayHasher<N>>> mBufs;
};

}

template<size_t N>
void FindAllOutputsBulk(EvalConfig::Settings const& settings, std::function<void()> func, std::span<Topology const> topologies, bs::BitSet<N>& out, EvalEngineConfig::Beta const& conf, BetaExtensions const& ext) {
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
            if (settings.secondOutput.isSingle || settings.secondOutput.isWellKnown) {
                s.enableSecondOutputFilter = true;
                if (settings.secondOutput.isWellKnown) {
                    s.wellKnown = true;
                } else {
                if constexpr (N == 1) {
                    s.expectedSecondOutput = {settings.secondOutput.single.x};
                } else if (N == 2) {
                    s.expectedSecondOutput = {settings.secondOutput.single.x, settings.secondOutput.single.y};
                } else {
                    assert(false);
                }

                }
            }
            if (conf.printTopology.enabled) {
                s.enablePrintTopology = true;
                s.topologyPrinter = ext.topologyPrinter;
                
                s.expectedOutput = {};
                if constexpr (N == 1) {
                    s.expectedOutput = {conf.printTopology.x};
                } else if (N == 2) {
                    s.expectedOutput = {conf.printTopology.x, conf.printTopology.y};
                } else {
                    assert(false);
                }
            }
            s.bits = settings.maxBits;
            if (settings.secondOutput.isWellKnown) {
                ExpensiveOutputCallback<N> cb {MaxIndex()};
                FindTopologyOutputsImpl<N>(s, t, cb, bufView, 0);
            } else {
                SimpleOutputCallback<N> cb {out};
                FindTopologyOutputsImpl<N>(s, t, cb, bufView, 0);
            }
        }
        if (func) {
            func();
        }
    }
}

template void FindAllOutputsBulk<1>(EvalConfig::Settings const&, std::function<void()>, std::span<Topology const>, bs::BitSet<1>&, EvalEngineConfig::Beta const& conf, BetaExtensions const& ext);
template void FindAllOutputsBulk<2>(EvalConfig::Settings const&, std::function<void()>, std::span<Topology const>, bs::BitSet<2>&, EvalEngineConfig::Beta const& conf, BetaExtensions const& ext);

}