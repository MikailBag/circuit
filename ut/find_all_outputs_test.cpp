#include "bruteforce/bruteforce.h"


#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <vector>

#include <log/log.h>


static Logger L = GetLogger("find-all-outputs-test");

TEST_CASE("Canonical signle digit encoder", "[find-all-outputs]") {
    SECTION("Basic test") {
        bruteforce::FindTopologyParams p;
        p.explicitNodeCountLimit = 1;
        std::vector<bruteforce::Topology> out = bruteforce::FindAllTopologies(p);
        CHECK(out.size() == 3);
        for (bruteforce::Topology const& t : out) {
            CHECK(t.size() == 1);
        }
    }
    SECTION("Finds outputs of trivial scheme") {
        bruteforce::FindOutputsParams p;
        p.bits = 2;
        std::vector<bruteforce::Topology> tps;
        tps.push_back(bruteforce::Topology{});
        std::vector<uint64_t> outputs = bruteforce::FindAllOutputs(p, tps);
        for (uint64_t x : outputs) {
            L().AttrU64("num", x).Log("Output");
        }
        CHECK(outputs.size() == 3);
        CHECK(std::find(outputs.begin(), outputs.end(), 1) != outputs.end());
        CHECK(std::find(outputs.begin(), outputs.end(), 2) != outputs.end());
        CHECK(std::find(outputs.begin(), outputs.end(), 4) != outputs.end());
    }
}
