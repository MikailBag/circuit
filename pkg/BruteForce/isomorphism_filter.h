#pragma once

#include "bruteforce/bruteforce.h"
#include "bruteforce/topology.h"

#include <span>
#include <vector>

namespace bf::iso {
void FilterTopologies(std::span<Topology const> input, FilterParams const& p, std::vector<Topology>& output);
}