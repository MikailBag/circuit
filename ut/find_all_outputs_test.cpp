#include "bruteforce/bruteforce.h"

#include <core/topology.h>

#include "log/log.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <vector>


static Logger L = GetLogger("bruteforce-test");

TEST_CASE("Exhaustive search", "[find-all-outputs]") {
    SECTION("Basic test") {
        bruteforce::FindTopologyParams p;
        p.explicitNodeCountLimit = 1;
        p.inputCount = 2;
        std::vector<bruteforce::Topology> out = bruteforce::FindAllTopologies(p);
        CHECK(out.size() == 3);
        for (bruteforce::Topology const& t : out) {
            CHECK(t.size() == 1);
        }
        p.inputCount = 1;
        out = bruteforce::FindAllTopologies(p);
        CHECK(out.size() == 1);
        for (bruteforce::Topology const& t : out) {
            CHECK(t.size() == 1);
        }
    }
    SECTION("Finds outputs of trivial scheme") {
        bruteforce::FindOutputsParams p;
        p.maxBits = 2;
        p.inputCount = 1;
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

    SECTION("Finds outpus of a bit more complex scheme") {
        bruteforce::FindOutputsParams p;
        p.maxBits = 2;
        p.maxExplicitNodeCount = 1;
        p.inputCount = 1;
        std::vector<bruteforce::Topology> tps;
        bruteforce::Topology tp;
        tp.push_back(TopologyNode{});
        tps.push_back(tp);
        std::vector<uint64_t> outputs = bruteforce::FindAllOutputs(p, tps);
        for (uint64_t x : outputs) {
            L().AttrU64("num", x).Log("Output");
        }
        CHECK(outputs.size() == 5);
        CHECK(std::find(outputs.begin(), outputs.end(), 0) != outputs.end());
        CHECK(std::find(outputs.begin(), outputs.end(), 1) != outputs.end());
        CHECK(std::find(outputs.begin(), outputs.end(), 2) != outputs.end());
        CHECK(std::find(outputs.begin(), outputs.end(), 3) != outputs.end());
        CHECK(std::find(outputs.begin(), outputs.end(), 4) != outputs.end());
    }
    SECTION("Correctly finds certain topology (regression)") {
        bruteforce::FindTopologyParams p;
        p.explicitNodeCountLimit = 1;
        p.inputCount = 2;
        std::vector<bruteforce::Topology> out = bruteforce::FindAllTopologies(p);
        bool ok = false;
        for (bruteforce::Topology const& t : out) {
            if (t[0].links[0] == 0 && t[0].links[1] == 0) {
                ok = true;
            }
        }
        CHECK(ok);
    }

    SECTION("Correctly finds output (regression)") {
        bruteforce::Topology tp;
        tp.push_back(TopologyNode{.links={0, 0}});
        bruteforce::FindOutputsParams p;
        p.inputCount = 2;
        p.maxBits = 3;
        p.maxExplicitNodeCount = 1;
        std::vector<bruteforce::Topology> tps;
        tps.push_back(tp);
        std::vector<uint64_t> outputs = bruteforce::FindAllOutputs(p, tps);
        bool ok = false;
        for (size_t i = 0; i < outputs.size(); i += 2) {
            if (outputs[i] == 3 && outputs[i+1] == 0) {
                ok = true;
            }
        }
        CHECK(ok);
    }
}
