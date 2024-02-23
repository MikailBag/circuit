#pragma once
#include <array>
#include <vector>
#include <string>
#include <cstdint>

inline constexpr uint8_t kShiftSignBit = 0x80;

struct Link {
    uint8_t index;
    uint8_t shiftWithSign;

    static Link OfAdd(size_t i, uint8_t shift);
    static Link OfSub(size_t i, uint8_t shift);

    bool IsNegated() const;
    uint8_t Shift() const;
};

struct GraphNode {
    std::array<Link, 2> links;

    GraphNode() = default;

    GraphNode(Link l, Link r): links({l, r}) {
    }
};

struct Graph {
    std::vector<GraphNode> nodes;
    Graph() = default;
    explicit Graph(Graph const&) = default;
    void operator=(Graph const&) = delete;
    Graph(Graph&&) = default;
    Graph& operator=(Graph&&) = default;
    
    static Graph Merge(size_t inputCount, Graph lhs, Graph const& rhs);
    bool IsValid(size_t inputCount) const;
    size_t ExplicitNodeCount() const {
        return nodes.size();
    }
};

std::vector<uint32_t> Evaluate1D(Graph const& g);
std::vector<std::pair<uint32_t, uint32_t>> Evaluate2D(Graph const& g);
std::vector<std::string> Describe(Graph const& g);