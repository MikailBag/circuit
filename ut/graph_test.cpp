#include "graph.h"
#include "graph_builder.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Graph evaluator", "[eval]") {
    Graph g = GraphBuilder()
        .Add(ByIndex(0).Shifted(0), ByIndex(0).Shifted(3)) // 1 + 8 = 9
        .Sub(ByIndex(1).Shifted(0), ByIndex(0).Shifted(2)) // 9 - 4 = 5
        .Build();
    std::vector<uint32_t> outs = Evaluate1D(g);

    REQUIRE(outs[0] == 1);
    REQUIRE(outs[1] == 9);
    REQUIRE(outs[2] == 5);
}
