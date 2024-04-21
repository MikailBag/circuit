#include "engine_launch.h"


#include "bitset.h"

#include "alpha/engine.h"
#include "beta/engine.h"

#include "log/log.h"

#include <cassert>
#include <concepts>
#include <thread>
#include <mutex>
#include <optional>
#include <stdexcept>

static logger::Logger L = logger::Get("bf.launch");

namespace bf {



namespace {
class Engine {
public:
    virtual ~Engine() = default;
    virtual void Run(std::span<Topology const> topologies, bs::BitSet<1>* out1, bs::BitSet<2>* out) const = 0;
};

template<std::invocable<std::span<Topology const>, bs::BitSet<1>*, bs::BitSet<2>*> F>
class FnEngine: public Engine {
public:
    explicit FnEngine(F&& fn) : mFn(std::move(fn)) {
    }

    void Run(std::span<Topology const> topologies, bs::BitSet<1>* out1, bs::BitSet<2>* out2) const override {
        mFn(topologies, out1, out2);
    }
private:
    F mFn;
};


class WorkScheduler {
public:
    WorkScheduler(size_t chunkSize, std::span<Topology const> input) : chunkSize(chunkSize), mRemaining(input) {
    }

    std::optional<std::span<Topology const>> NextTask() {
        std::lock_guard lk {mMu};
        if (mRemaining.empty()) {
            return {};
        }
        size_t cntToReturn = std::min(mRemaining.size(), chunkSize);
        std::span<Topology const> ret = mRemaining.subspan(0, cntToReturn);
        mRemaining = mRemaining.subspan(cntToReturn);
        return ret;
    }
private:
    size_t chunkSize;
    std::mutex mMu;
    std::span<Topology const> mRemaining;
};

void InvokeEngineImpl(Engine const& engine, std::vector<Topology> const& topologies, LaunchConfig const& lp, bs::BitSet<1>* out1, bs::BitSet<2>* out2) {
    if (!lp.parallel) {
        L().AttrU64("tasks", topologies.size()).Log("Starting single-thread engine");
    
        engine.Run(topologies, out1, out2);
        return;
    }
    L().AttrU64("threads", lp.threadCount).AttrU64("chunkSize", lp.chunkSize).AttrU64("tasks", topologies.size()).Log("Starting parallel engine");
    WorkScheduler sched(lp.chunkSize, topologies);
    std::vector<std::jthread> workers;
    for (size_t i = 0; i < lp.threadCount; i++) {
        workers.emplace_back([sched = &sched](std::stop_token stop){
            while (!stop.stop_requested()) {
                std::optional<std::span<Topology const>> items = sched->NextTask();
                if (!items) {
                    break;
                }
            }
        });
    }
    for (size_t i = 0; i < lp.threadCount; i++) {
        workers[i].join();
    }
}
}

void InvokeEngine(EngineParams const& ep, LaunchConfig const& lp, std::vector<Topology> const& topologies, bs::BitSet<1>* out1, bs::BitSet<2>* out2) {
    if (ep.inputCount == 1) {
        assert(out1 != nullptr);
        if (ep.config.isAlpha) {
            InvokeEngineImpl(FnEngine{[ep = &ep](std::span<Topology const> topologies, bs::BitSet<1>* out1, [[maybe_unused]] bs::BitSet<2>* out2){
                alpha::FindAllOutputsBulk<1>(ep->maxBits, ep->maxExplicitNodeCount, ep->progressListener, topologies, *out1);
            }}, topologies, lp, out1, out2);
            
        } else if (ep.config.isBeta) {
            InvokeEngineImpl(FnEngine{[ep = &ep](std::span<Topology const> topologies, bs::BitSet<1>* out1, [[maybe_unused]] bs::BitSet<2>* out2){
                beta::FindAllOutputsBulk<1>(ep->maxBits, ep->progressListener, topologies, *out1);
            }}, topologies, lp, out1, out2);
        }
    } else if (ep.inputCount == 2) {
        assert(out2 != nullptr);
        if (ep.config.isAlpha) {
            InvokeEngineImpl(FnEngine{[ep = &ep](std::span<Topology const> topologies, [[maybe_unused]] bs::BitSet<1>* out1, bs::BitSet<2>* out2){
                alpha::FindAllOutputsBulk<2>(ep->maxBits, ep->maxExplicitNodeCount, ep->progressListener, topologies, *out2);
            }}, topologies, lp, out1, out2);
            
        } else if (ep.config.isBeta) {
            InvokeEngineImpl(FnEngine{[ep = &ep](std::span<Topology const> topologies, [[maybe_unused]] bs::BitSet<1>* out1, bs::BitSet<2>* out2) {
                beta::FindAllOutputsBulk<2>(ep->maxBits, ep->progressListener, topologies, *out2);
            }}, topologies, lp, out1, out2);
        }
    } else {
        std::abort();
    }
}

namespace {
size_t GetSize(uint8_t maxBits) {
    return static_cast<size_t>(1) << static_cast<size_t>(maxBits);
}
}

bs::BitSet<1> PrepareBitset1(uint8_t maxBits) {
    size_t sz = GetSize(maxBits);
    return bs::BitSet{std::array{sz+1}};
}
bs::BitSet<2> PrepareBitset2(uint8_t maxBits) {
    size_t sz = GetSize(maxBits);
    return bs::BitSet{std::array{sz+1, sz+1}};
}
}