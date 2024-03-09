#pragma once

#include "graph.h"

namespace bruteforce::detail {
struct IsomorphismKey {};

IsomorphismKey Get(Graph const& g);
}