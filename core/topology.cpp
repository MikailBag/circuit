#include "core/topology.h"

std::vector<std::string> DescribeTopology(std::span<TopologyNode const> topology, size_t inputCount) {
    std::vector<std::string> out;
    for (size_t i = 0; i < inputCount; i++) {
        std::string res;
        res += "[";
        res += std::to_string(i);
        res += "] = (input)";
        out.push_back(std::move(res));
    }
    size_t counter = 0;
    for (TopologyNode const& n : topology) {
        auto i = static_cast<size_t>(n.links[0]);
        auto j = static_cast<size_t>(n.links[1]);
        std::string res = "[";
        res += std::to_string(counter + inputCount);
        res += "] = [";
        res += std::to_string(i);
        res += "] + [";
        res += std::to_string(j);
        res += "]";
        out.push_back(std::move(res));
        ++counter;
    }
    return out;
}