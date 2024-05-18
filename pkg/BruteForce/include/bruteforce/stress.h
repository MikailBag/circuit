#pragma once

#include "params.h"
#include "bruteforce/topology.h"

#include <optional>
#include <vector>

namespace bf::stress {
struct Mismatch {
    size_t topologyIndex = SIZE_MAX;
    std::vector<uint64_t> output;
    bool leftResult = false;
    bool rightResult = false;
};
[[nodiscard]]
std::optional<Mismatch> Compare(EvalParams const& left, EvalParams const& right, std::vector<Topology> const& topologies);
}