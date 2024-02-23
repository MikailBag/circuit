#include "csd_solver.h"

#include "csd.h"

#include "simple_builder.h"

#include "log.h"

#include <cassert>

#include <iostream>

static Logger L = GetLogger("csd");



Graph MakeGraphUsingCsd(uint64_t x) {
    L().AttrU64("input", x).Log("Starting solve");
    CanonicalSignedDigitRepresentation repr = EncodeCsd(x);

    SimpleBuilder g;
    std::vector<size_t> positiveComponents;
    std::vector<size_t> negativeComponents;
    std::vector<SimpleBuilder::Dep> deps;
    for (uint8_t bit = 0; bit < 64; ++bit) {
        CanonicalSignedDigitRepresentation::Kind k = repr.Classify(bit);
        if (k == CanonicalSignedDigitRepresentation::Kind::NONE) {
            continue;
        }
        L().AttrU64("bit", bit).AttrI64("sign", (k == CanonicalSignedDigitRepresentation::Kind::POSITIVE ? 1 : -1)).Log("Adding bit");
        deps.push_back({
            .input=1,
            .sign=(k == CanonicalSignedDigitRepresentation::Kind::POSITIVE ? SimpleBuilder::Sign::POSITIVE : SimpleBuilder::Sign::NEGATIVE),
            .shift = bit
        });
    }
    g.Add(std::move(deps));
    auto res = std::move(g).Finish();
    assert(res.IsValid(1));
    return res;
}

namespace {
class CsdSolver : public Solver {
public:
    Graph Solve(std::span<uint64_t> x) override {
        auto g = MakeGraphUsingCsd(x[0]);
        for (size_t i = 1; i < x.size(); ++i) {
            g = Graph::Merge(1, std::move(g), MakeGraphUsingCsd(x[i]));
        }
        return g;
    }

    std::string_view Name() const override {
        return "CSD";
    }
};
}

std::unique_ptr<Solver> MakeCsdSolver() {
    return std::make_unique<CsdSolver>();
}