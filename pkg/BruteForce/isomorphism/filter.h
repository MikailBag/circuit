#pragma once

#include "bruteforce/bruteforce.h"
#include "bruteforce/topology.h"

#include <span>
#include <vector>

namespace bf::iso {

enum class Order {
    kGroupFirst,
    kNormal
};

struct OrderedTopology {
    Topology tp;
    Order ord;

    friend auto operator<=>(OrderedTopology const& lhs, OrderedTopology const& rhs) = default;
};
void FilterTopologies(std::span<Topology const> input, FilterParams const& p, std::vector<OrderedTopology>& output);
}