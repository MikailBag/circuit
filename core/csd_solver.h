#pragma once
#include "graph.h"
#include "solver.h"

#include <memory>

Graph MakeGraphUsingCsd(uint64_t x);

std::unique_ptr<Solver> MakeCsdSolver();