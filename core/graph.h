#pragma once
#include <array>
#include <vector>
#include <string>
#include <cstdint>

inline constexpr uint8_t kShiftSignBit = 0x80;

struct Link {
    uint8_t index;
    uint8_t shiftWithSign;

    bool IsNegated() const;
    uint8_t Shift() const;
};

struct GraphNode {
    std::array<Link, 2> links;
};

struct Graph {
    std::vector<GraphNode> nodes;
    Graph() = default;
    Graph(Graph const&) = delete;
    void operator=(Graph const&) = delete;
    Graph(Graph&&) = default;
    Graph& operator=(Graph&&) = default;
    
    static Graph Merge(Graph lhs, Graph const& rhs);
    bool IsValid() const;
    size_t NodeCount() const {
        return nodes.size() + 1;
    }
};

std::vector<uint32_t> Evaluate(Graph const& g);
std::vector<std::string> Describe(Graph const& g);