#include "find_all_outputs.h"


#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>


TEST_CASE("Canonical signle digit encoder", "[find-all-outputs]") {
    SECTION("Basic test") {
        find_all_outputs::FindTopologyParams p;
        p.explicitNodeCountLimit = 1;
        std::vector<find_all_outputs::Topology> out = find_all_outputs::FindAllTopologies(p);
        CHECK(out.size() == 3);
        for (find_all_outputs::Topology const& t : out) {
            CHECK(t.size() == 1);
        }
    }
}
