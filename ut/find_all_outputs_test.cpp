#include "bruteforce/bruteforce.h"

#include <core/topology.h>

#include "log/log.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <vector>


static Logger L = GetLogger("bruteforce-test");

    TEST_CASE("Basic test", "[bruteforce][bruteforce/outputs]") {
        bf::FindTopologyParams p;
        p.explicitNodeCountLimit = 1;
        p.inputCount = 2;
        std::vector<bf::Topology> out = bf::FindAllTopologies(p);
        CHECK(out.size() == 3);
        for (bf::Topology const& t : out) {
            CHECK(t.size() == 1);
        }
        p.inputCount = 1;
        out = bf::FindAllTopologies(p);
        CHECK(out.size() == 1);
        for (bf::Topology const& t : out) {
            CHECK(t.size() == 1);
        }
    }
    TEST_CASE("Finds outputs of trivial scheme", "[bruteforce][bruteforce/outputs]") {
        bf::FindOutputsParams p;
        p.maxBits = 2;
        p.inputCount = 1;
        std::vector<bf::Topology> tps;
        tps.push_back(bf::Topology{});
        std::vector<uint64_t> outputs = bf::FindAllOutputs(p, tps);
        for (uint64_t x : outputs) {
            L().AttrU64("num", x).Log("Output");
        }
        CHECK(outputs.size() == 3);
        CHECK(std::find(outputs.begin(), outputs.end(), 1) != outputs.end());
        CHECK(std::find(outputs.begin(), outputs.end(), 2) != outputs.end());
        CHECK(std::find(outputs.begin(), outputs.end(), 4) != outputs.end());
    }

    TEST_CASE("Finds outpus of a bit more complex scheme", "[bruteforce][bruteforce/outputs]") {
        bf::FindOutputsParams p;
        p.maxBits = 2;
        p.maxExplicitNodeCount = 1;
        p.inputCount = 1;
        std::vector<bf::Topology> tps;
        bf::Topology tp;
        tp.push_back(TopologyNode{});
        tps.push_back(tp);
        std::vector<uint64_t> outputs = bf::FindAllOutputs(p, tps);
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
    TEST_CASE("Correctly finds certain topology (regression)", "[bruteforce][bruteforce/outputs][regression]") {
        bf::FindTopologyParams p;
        p.explicitNodeCountLimit = 1;
        p.inputCount = 2;
        std::vector<bf::Topology> out = bf::FindAllTopologies(p);
        bool ok = false;
        for (bf::Topology const& t : out) {
            if (t[0].links[0] == 0 && t[0].links[1] == 0) {
                ok = true;
            }
        }
        CHECK(ok);
    }

TEST_CASE("Correctly finds output", "[bruteforce][bruteforce/outputs][regression]") {
    bf::Topology tp;
    tp.push_back(TopologyNode{.links={0, 0}});
    bf::FindOutputsParams p;
    p.inputCount = 2;
    p.maxBits = 3;
    p.maxExplicitNodeCount = 1;
    std::vector<bf::Topology> tps;
    tps.push_back(tp);
    std::vector<uint64_t> outputs = bf::FindAllOutputs(p, tps);
    bool ok = false;
    for (size_t i = 0; i < outputs.size(); i += 2) {
        if (outputs[i] == 3 && outputs[i+1] == 0) {
            ok = true;
        }
    }
    CHECK(ok);
}
