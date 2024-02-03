#include "simple_solver.h"

#include "min_adder_depth.h"
#include "simple_builder.h"

#include "log.h"

#include <algorithm>
#include <string>
#include <vector>


namespace {
Logger L = GetLogger("simple-solver");

class SimpleSolver : public Solver {
public:
    Graph Solve(std::span<uint64_t> rawTargets) override {
        std::vector<uint64_t> targets {rawTargets.begin(), rawTargets.end()};
        std::sort(targets.begin(), targets.end(), [](uint64_t a, uint64_t b) {
            return MinAdderDepth(a) < MinAdderDepth(b);
        });
        SimpleBuilder b;
        std::vector<uint64_t> parts; // TODO: use NOFs too
        parts.push_back(1);
        for (uint64_t const target : targets) {
            L().AttrU64("target", target).Log("Reaching target");
            uint64_t remaining = target;
            std::vector<SimpleBuilder::Dep> deps;
            for (uint64_t const p : parts) {
                while (remaining >= p) {
                    uint8_t s = 0;
                    uint64_t p2 = p;
                    while (p2*2 <= remaining) {
                        p2 *= 2;
                        ++s;
                    }
                    deps.push_back(SimpleBuilder::Dep{.input=p,.sign=SimpleBuilder::Sign::POSITIVE,.shift=s});
                    L().AttrU64("part", p).AttrU64("val", p2).Log("Part");
                    remaining -= p2;
                }
            }
            b.Add(std::move(deps));
            parts.insert(parts.begin(), target);
        }
        
        return std::move(b).Finish();
    }

    std::string_view Name() const override {
        return "Simple";
    }
};
}


std::unique_ptr<Solver> MakeSimpleSolver() {
    return std::make_unique<SimpleSolver>();
}