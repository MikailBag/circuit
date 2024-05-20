#pragma once

#include <core/graph.h>

#include <functional>

namespace bf {
struct BetaExtensions {
    std::function<void(Graph)> topologyPrinter;
};
}