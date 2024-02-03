#include "all_solvers.h"

#include "csd_solver.h"
#include "simple_solver.h"


std::vector<std::unique_ptr<Solver>> BuildAllSolvers() {
    std::vector<std::unique_ptr<Solver>> res;
    res.push_back(MakeCsdSolver());
    res.push_back(MakeSimpleSolver());
    return res;
}
