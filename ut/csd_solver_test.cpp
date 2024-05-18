#include "csd_solver.h"
#include "core/graph.h"
#include "solver_bounds_test_base.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <iostream>


TEST_CASE("CSD solver", "[solver-csd]") {
    SECTION("Is correct") {
        uint64_t n = GENERATE(37, 147, 15, 11, 415);
        Graph g = MakeGraphUsingCsd(n);
        CHECK(Evaluate1D(g).back() == n);
    }
}
