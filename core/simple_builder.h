#pragma once

#include "core/graph.h"

#include <vector>
#include <cstdint>
#include <cstdlib>

class SimpleBuilder {
public:
    SimpleBuilder();
    enum class Sign {
        POSITIVE,
        NEGATIVE
    };
    struct Dep {
        uint64_t input;
        Sign sign;        
        uint8_t shift;
    };
    uint64_t Add(std::vector<Dep>&& deps);
    Graph Finish() &&;
private:
    std::vector<std::vector<Dep>> mNodeSpecs;
};