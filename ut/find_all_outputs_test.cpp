#include "bruteforce/bruteforce.h"
#include "bruteforce/config.h"

#include <core/topology.h>

#include "log/log.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <vector>


static logger::Logger L = logger::Get("bruteforce-test");

static bf::EvalConfig CONFIG_ALPHA = []{
    bf::EvalConfig c;
    c.engine.isAlpha = true;
    return c;
}();

static bf::EvalConfig CONFIG_BETA = []{
    bf::EvalConfig c;
    c.engine.isBeta = true;
    return c;
}();

TEST_CASE("Basic test", "[bruteforce][bruteforce/list]") {
    bf::FindTopologyParams p;
    p.explicitNodeCountLimit = 1;
    p.inputCount = 2;
    std::vector<bf::Topology> out = bf::FindAllTopologies(p);
    CHECK(out.size() == 3);
    for (bf::Topology const& t : out) {
        CHECK(t.nodes.size() == 1);
    }
    p.inputCount = 1;
    out = bf::FindAllTopologies(p);
    CHECK(out.size() == 1);
    for (bf::Topology const& t : out) {
        CHECK(t.nodes.size() == 1);
    }
}
TEST_CASE("Finds outputs of trivial scheme", "[bruteforce][bruteforce/outputs]") {
    bf::EvalConfig outConf = GENERATE(CONFIG_ALPHA, CONFIG_BETA);
    outConf.settings.maxBits = 2;
    bf::LaunchConfig lc;
    bf::FindOutputsParams p {outConf, lc};
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

TEST_CASE("Finds outputs of a bit more complex scheme", "[bruteforce][bruteforce/outputs]") {
    bf::EvalConfig outConf = GENERATE(CONFIG_ALPHA, CONFIG_BETA);
    outConf.settings.maxBits = 2;
    bf::LaunchConfig lc;
    bf::FindOutputsParams p {outConf, lc};
    p.maxExplicitNodeCount = 1;
    p.inputCount = 1;
    std::vector<bf::Topology> tps;
    bf::Topology tp;
    TopologyNode tn;
    tn.links[0] = 0;
    tn.links[1] = 0;
    tp.nodes.push_back(tn);
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

TEST_CASE("Finds two-output topology", "[bruteforce][bruteforce/outputs][bruteforce/outputs/second]") {
    bf::EvalConfig evalConf = CONFIG_BETA;
    evalConf.settings.maxBits = 4;
    evalConf.settings.secondOutput.isEnabled = true;
    evalConf.settings.secondOutput.enabled.x = 5;
    bf::LaunchConfig lc;
    bf::FindOutputsParams p {evalConf, lc};
    p.maxExplicitNodeCount = 2;
    p.inputCount = 1;
    /*
    x = (1 + 1 << 2) = 5
    y = (x<<1 + 1) = 11
    */
    std::vector<bf::Topology> tps;
    bf::Topology tp;
    TopologyNode tn;
    tn.links[0] = 0;
    tn.links[1] = 0;
    tp.nodes.push_back(tn);
    tn.links[0] = 0;
    tn.links[1] = 1;
    tp.nodes.push_back(tn);
    tps.push_back(tp);
    std::vector<uint64_t> outputs = bf::FindAllOutputs(p, tps);
    for (uint64_t x : outputs) {
        L().AttrU64("num", x).Log("Output");
    }
    CHECK(std::find(outputs.begin(), outputs.end(), 8) != outputs.end());
}

TEST_CASE("Respects two-output filter", "[bruteforce][bruteforce/outputs][bruteforce/outputs/second]") {
    bf::EvalConfig evalConf = CONFIG_BETA;
    evalConf.settings.maxBits = 5;
    evalConf.settings.secondOutput.isEnabled = true;
    evalConf.settings.secondOutput.enabled.x = 21;
    bf::LaunchConfig lc;
    bf::FindOutputsParams p {evalConf, lc};
    p.maxExplicitNodeCount = 2;
    p.inputCount = 1;
    std::vector<bf::Topology> tps;
    bf::Topology tp;
    TopologyNode tn;
    tn.links[0] = 0;
    tn.links[1] = 0;
    tp.nodes.push_back(tn);
    tn.links[0] = 0;
    tn.links[1] = 1;
    tp.nodes.push_back(tn);
    tps.push_back(tp);
    std::vector<uint64_t> outputs = bf::FindAllOutputs(p, tps);
    for (uint64_t x : outputs) {
        L().AttrU64("num", x).Log("Output");
    }
    CHECK(std::find(outputs.begin(), outputs.end(), 7) == outputs.end());
}

TEST_CASE("Correctly finds certain topology (regression)", "[bruteforce][bruteforce/outputs][regression]") {
    bf::FindTopologyParams p;
    p.explicitNodeCountLimit = 1;
    p.inputCount = 2;
    std::vector<bf::Topology> out = bf::FindAllTopologies(p);
    bool ok = false;
    for (bf::Topology const& t : out) {
        if (t.nodes[0].links[0] == 0 && t.nodes[0].links[1] == 0) {
            ok = true;
        }
    }
    CHECK(ok);
}

TEST_CASE("Correctly finds output", "[bruteforce][bruteforce/outputs][regression]") {
    bf::EvalConfig outConf = GENERATE(CONFIG_ALPHA, CONFIG_BETA);
    outConf.settings.maxBits = 3;
    bf::LaunchConfig lc;
    bf::FindOutputsParams p {outConf, lc};
    bf::Topology tp;
    tp.nodes.push_back(TopologyNode{.links={0, 0}});
    p.inputCount = 2;
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
