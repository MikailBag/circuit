#include "simple_builder.h"
#include "core/graph.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <iostream>


TEST_CASE("Simple builder", "[builder-simple]") {
    SECTION("Is correct [N=37]") {
        SimpleBuilder b;
        std::vector<SimpleBuilder::Dep> deps = {
            {.input=1, .sign=SimpleBuilder::Sign::POSITIVE, .shift=0},
            {.input=1, .sign=SimpleBuilder::Sign::POSITIVE, .shift=2},
            {.input=1, .sign=SimpleBuilder::Sign::POSITIVE, .shift=5}
        };
        b.Add(std::move(deps));
        Graph g = std::move(b).Finish();
        CHECK(Evaluate1D(g).back() == 37);
    }
    SECTION("Is correct [N=33]") {
        SimpleBuilder b;
        std::vector<SimpleBuilder::Dep> deps = {
            {.input=1, .sign=SimpleBuilder::Sign::POSITIVE, .shift=0},
            {.input=1, .sign=SimpleBuilder::Sign::POSITIVE, .shift=5}
        };
        b.Add(std::move(deps));
        Graph g = std::move(b).Finish();
        CHECK(Evaluate1D(g).back() == 33);
    }

    SECTION("Is correct [N=5]") {
        SimpleBuilder b;
        std::vector<SimpleBuilder::Dep> deps = {
            {.input=1, .sign=SimpleBuilder::Sign::POSITIVE, .shift=0},
            {.input=1, .sign=SimpleBuilder::Sign::POSITIVE, .shift=2}
        };
        b.Add(std::move(deps));
        Graph g = std::move(b).Finish();
        CHECK(Evaluate1D(g).back() == 5);
    }
}
