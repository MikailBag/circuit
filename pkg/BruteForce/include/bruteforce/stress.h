#pragma once

#include "params.h"
#include "bruteforce/topology.h"

#include <optional>
#include <vector>

namespace bf {
std::optional<size_t> Compare(FindOutputsParams const& left, FindOutputsParams const& right, std::vector<Topology> const& topologies);
}