#pragma once

#include "graph.h"

#include <vector>


inline constexpr size_t kInCount = 2;

struct Params {
    size_t explicitNodeCountLimit;
};

void FindAllTopologies(Params const& p, std::vector<Graph>& out);