#include "simple_builder.h"

#include "core/graph_builder.h"

#include "log/log.h"

#include <stdexcept>
#include <string>
#include <cassert>
#include <unordered_map>
#include <vector>

using namespace std::string_literals;



static logger::Logger L = logger::Get("simple-builder");

SimpleBuilder::SimpleBuilder() = default;

auto SimpleBuilder::Add(std::vector<Dep>&& deps) -> size_t {
    if (deps.empty()) {
        throw std::invalid_argument("invalid empty dependency list");
    }
    uint64_t newValue = 0;
    for (size_t i = 0; i < deps.size(); ++i) {
        newValue += (deps[i].input << deps[i].shift);
    }
    mNodeSpecs.push_back(std::move(deps));
    return newValue;
    /*
    int64_t newValue = 0;
    for (size_t i = 0; i < deps.size(); ++i) {
        if (deps[i].idx >= mNodes.size()) {
            throw std::invalid_argument("invalid node reference "s + std::to_string(deps[i].idx));
        }
        newValue += (deps[i].input << deps[i].shift);
    }
    Node n;
    n.deps = std::move(deps);
    n.value = newValue;
    size_t sz = mNodes.size();
    mNodes.push_back(std::move(n));
    return sz;*/
}

namespace {
/*
struct Node {
    int64_t value;
    std::vector<GraphBuilder::Dep> deps;
};



*/

struct NodeDep {
    size_t idx;
    SimpleBuilder::Sign sign;        
    uint8_t shift;
};

struct Node {
    std::vector<NodeDep> deps;
    uint64_t output;
};

struct BuildState {
    GraphBuilder g;
    std::vector<size_t> outputs;
    std::vector<int64_t> values;
    std::vector<bool> negated;
    explicit BuildState(size_t nodeCount) : g(1) {
        outputs.reserve(nodeCount);
        negated.reserve(nodeCount);
        values.reserve(nodeCount);

        //outputs.push_back(0);
        //negated.push_back(false);
        //values.push_back(1);
    }

    void Append(Node&& node) {
        L().AttrU64("expected", node.output).AttrU64("depsCount", node.deps.size()).Log("Adding node");
        for (NodeDep const& d : node.deps) {
            L()
                .AttrU64("index", d.idx)
                .AttrU64("shift", d.shift)
                .AttrI64("sign", (d.sign == SimpleBuilder::Sign::POSITIVE ? 1 : -1))
                .Log("Dependency");
        }
        assert(outputs.size() == negated.size());
        assert(outputs.size() == values.size());
        assert(node.deps[0].idx < outputs.size() || node.deps[0].idx == SIZE_MAX);
        size_t cur;
        int64_t curValue;
        if (node.deps[0].idx == SIZE_MAX) {
            cur = 0;
            curValue = 1;
        } else {
            cur = outputs[node.deps[0].idx];
            curValue = values[node.deps[0].idx];
        }
        curValue <<= node.deps[0].shift;
        L().AttrI64("initialValue", curValue).AttrU64("index", cur).Log("Processed initial dependency");
        bool curNegated = node.deps[0].sign == SimpleBuilder::Sign::NEGATIVE;
        uint8_t shift = node.deps[0].shift;
        for (size_t j = 1; j < node.deps.size(); ++j) {
            L()
                .AttrU64("depIndex", j)
                .AttrU64("inIndex", node.deps[j].idx)
                .AttrU64("shift", node.deps[j].shift)
                .Log("Processing dependency");
            //assert(outputs.size() == j);
            //assert(cur < outputs.size());
            assert(node.deps[j].idx < outputs.size() || node.deps[j].idx == SIZE_MAX);
            size_t next;
            if (node.deps[j].idx == SIZE_MAX) {
                next = 0;
            } else {
                next = outputs[node.deps[j].idx];
            }
            assert(cur < g.NodeCount() || (j == 1 && cur == SIZE_MAX));
            assert(next < g.NodeCount());
            //assert(next < outputs.size());
            int64_t valueDelta;
            if (node.deps[j].idx == SIZE_MAX) {
                valueDelta = 1;
            } else {
                valueDelta = values[node.deps[j].idx];
            }
            valueDelta <<= node.deps[j].shift;
            L().AttrI64("delta", valueDelta).Log("Dependency delivers delta");
            bool nextIsNeg = node.deps[j].sign == SimpleBuilder::Sign::NEGATIVE;
            GraphBuilder::LinkB curLink = ByIndex(cur).Shifted(shift);
            GraphBuilder::LinkB chLink = ByIndex(next).Shifted(node.deps[j].shift);

            if (curNegated == nextIsNeg) {
                g = std::move(g).Add(curLink, chLink);
                curValue += valueDelta;
            } else if (curValue >= valueDelta) {
                g = std::move(g).Sub(curLink, chLink);
                curValue -= valueDelta;
            } else {
                g = std::move(g).Sub(chLink, curLink);
                curNegated = !curNegated;
                curValue = valueDelta - curValue;
            }
            cur = g.NodeCount() - 1;
            shift = 0;
            L()
                .AttrU64("depIndex", j)
                .AttrU64("curIndex", cur)
                .AttrU64("value", curValue)
                .Log("Processed dependency");
        }
        L().AttrU64("actualValue", curValue).AttrU64("index", cur).Log("Node finished");
        assert(static_cast<uint64_t>(curValue) == node.output);
        values.push_back(curValue);
        outputs.push_back(cur);
        negated.push_back(curNegated);
    }
};

void Dfs(std::vector<std::vector<size_t>> const& graph, size_t v, std::vector<bool>& used, std::vector<size_t>& order) {
    used[v] = true;
    for (size_t w : graph[v]) {
        if (used[w]) {
            continue;
        }
        Dfs(graph, w, used, order);
    }
    order.push_back(v);
}

uint64_t Eval(std::vector<SimpleBuilder::Dep> const spec) {
    uint64_t output = 0;
    for (SimpleBuilder::Dep const& d : spec) {
        uint64_t delta = d.input << d.shift;
        if (d.sign == SimpleBuilder::Sign::POSITIVE) {
            output += delta;
        }
    }
    for (SimpleBuilder::Dep const& d : spec) {
        uint64_t delta = d.input << d.shift;
        if (d.sign == SimpleBuilder::Sign::NEGATIVE) {
            output -= delta;
        }
    }
    return output;
}
}

auto SimpleBuilder::Finish() && -> Graph {
    std::unordered_map<uint64_t, size_t> nodeIdxs;
    
    std::vector<bool> skip(mNodeSpecs.size());
    for (size_t i = 0; i < mNodeSpecs.size(); ++i) {
        uint64_t output = Eval(mNodeSpecs[i]);
        L().AttrU64("value", output).Log("Registering fundamental");
        if (nodeIdxs.count(output)) {
            size_t j = nodeIdxs[output];
            L().AttrU64("prev", j).Log("Dropping old node");
            skip[j] = true;
        }
        nodeIdxs[output] = i;
    }
    L().Log("Building graph");
    std::vector<std::vector<size_t>> graph(mNodeSpecs.size());
    for (size_t i = 0; i < mNodeSpecs.size(); ++i) {
        if (skip[i]) {
            continue;
        }
        graph[i].reserve(mNodeSpecs[i].size());
        for (Dep const& d : mNodeSpecs[i]) {
            if (d.input == 1) {
                continue;
            }
            if (nodeIdxs.count(d.input) == 0) {
                throw std::invalid_argument("unknown dependency "s + std::to_string(d.input));
            }
            size_t j = nodeIdxs.at(d.input);
            graph[i].push_back(j);
        }
    }
    L().Log("Generating order");
    std::vector<bool> used(mNodeSpecs.size());
    std::vector<size_t> order;
    order.reserve(mNodeSpecs.size());
    for (size_t i = 0; i < mNodeSpecs.size(); ++i) {
        if (used[i] || skip[i]) {
            continue;
        }
        Dfs(graph, i, used, order);
    }
    std::vector<size_t> orderRev(mNodeSpecs.size());
    for (size_t i = 0; i < mNodeSpecs.size(); ++i) {
        if (skip[i]) {
            continue;
        }
        orderRev[order[i]] = i;
    }

    L().Log("Preparing nodes");
    std::vector<Node> nodes;
    for (size_t i = 0; i < mNodeSpecs.size(); ++i) {
        if (skip[i]) {
            continue;
        }
        std::vector<Dep> const& deps = mNodeSpecs[order[i]];
        Node n;
        n.output = Eval(deps);
        n.deps.reserve(deps.size());
        for (Dep const& d: deps) {
            NodeDep nd;
            if (d.input == 1) {
                nd.idx = SIZE_MAX;
            } else {
                nd.idx = orderRev[nodeIdxs.at(d.input)];
            }
            nd.shift = d.shift;
            nd.sign = d.sign;
            n.deps.push_back(nd);
        }
        nodes.push_back(std::move(n));
    }
    L().Log("Building graph");
    BuildState st(nodes.size());
    for (Node& n : nodes) {
        st.Append(std::move(n));
    }
    Graph result = std::move(st.g).Build();
    assert(result.IsValid(1));
    return result;
}