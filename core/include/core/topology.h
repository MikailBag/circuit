#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

struct TopologyNode {
    std::array<uint8_t, 2> links;

    friend std::strong_ordering operator<=>(TopologyNode const& lhs, TopologyNode const& rhs) = default;

};


std::vector<std::string> DescribeTopology(std::span<TopologyNode const> topology, size_t inputCount);