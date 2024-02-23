#pragma once

#include "graph.h"

namespace find_all_outputs::detail {
struct IsomorphismKey {};

IsomorphismKey Get(Graph const& g);
}