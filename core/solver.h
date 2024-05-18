#pragma once

#include "core/graph.h"

#include <string_view>
#include <span>

class Solver {
public:
    virtual Graph Solve(std::span<uint64_t> goal) = 0;
    virtual std::string_view Name() const = 0;

    virtual ~Solver() = default;
};