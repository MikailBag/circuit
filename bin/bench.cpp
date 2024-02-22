#include "solver.h"
#include "all_solvers.h"

#include <unordered_set>
#include <iostream>

constexpr static size_t kFail = SIZE_MAX;

struct Test {
    std::unordered_set<uint64_t> goal;
    uint64_t bound;
};


static uint64_t RunSolverOnTest(Solver& s, Test const& t) {
    auto goal = t.goal;
    std::vector<uint64_t> goalVec;
    for (uint64_t x : goal) {
        goalVec.push_back(x);
    }

    Graph g = s.Solve({goalVec.data(), goalVec.size()});

    auto f = Evaluate1D(g);
    for (auto x : f) {
        goal.erase(x);
    }
    if (!goal.empty()) {
        std::cout << "Graph:\n";
        auto desc = Describe(g);
        for (auto const& s : desc) {
            std::cout << " - " << s << '\n';
        }
        std::cout << "Fundamentals:\n";
        for (auto x : f) {
            std::cout << x << ' ';
        }
        std::cout << '\n';
        return kFail; 
    }
    return g.NodeCount();
}

static void RunTest(std::vector<std::unique_ptr<Solver>> const& solvers, Test const& t) {
    std::cout << "--- TEST ---\n";
    std::cout << "Goals: ";
    for (uint64_t x : t.goal) {
        std::cout << x << ' ';
    }
    std::cout << '\n';
    uint64_t opt = SIZE_MAX;
    for (std::unique_ptr<Solver> const& s : solvers) {
        uint64_t res = RunSolverOnTest(*s, t);
        std::cout << s->Name() << ": ";
        if (res == kFail) {
            std::cout << "failed";
        } else {
            std::cout << res;
            opt = std::min(opt, res);
        }
        std::cout << '\t';
    }
    std::cout << '\n';
    if (opt < t.bound) {
        std::cout << "THIS IS LESS THAN BOUND " << t.bound << '\n';
    }
}

static std::vector<Test> PrepareTests() {
    std::vector<Test> tests;
    tests.push_back(Test{
        .goal={3,5,7},
        .bound=4
    });
    tests.push_back(Test{
        .goal={537,235,485},
        .bound=0
    });
    tests.push_back(Test{
        .goal={4575,5035,5953},
        .bound=0
    });

    tests.push_back(Test{
        .goal={578,47,4575,5035,5953},
        .bound=0
    });

    tests.push_back(Test{
        .goal={578,47,4575,5035,5953,4638,375,27,535,385,387,389,391,393,395,397},
        .bound=0
    });


    return tests;
}

int main() {
    std::vector<std::unique_ptr<Solver>> solvers = BuildAllSolvers();
    std::vector<Test> tests = PrepareTests();
    std::cout << "Will run " << solvers.size() << " solvers on " << tests.size() << " tests\n";
    for (Test const& t : tests) {
        RunTest(solvers, t);
    }
}
