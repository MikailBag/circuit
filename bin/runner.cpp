#include "argparser.h"
#include "solver.h"
#include "csd_solver.h"

#include <string>
#include <iostream>



int main(int argc, char** argv) {
    argparser::ParserConfig pc;
    pc.AddOption("target").Required();
    std::map<std::string, std::string> opts = argparser::ParseArgv(pc, argc, argv);
    uint64_t x = std::stoi(opts["target"]);
    Graph g = MakeGraphUsingCsd(x);
    if (Evaluate1D(g).back() != x) {
        std::cerr << "Internal error\n";
        exit(1);
    }
    std::cout << "OK\n";
    std::cout << "Used nodes " << g.nodes.size() << "\n";
}