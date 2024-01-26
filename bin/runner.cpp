#include <argv.h>
#include <solver.h>
#include <csd_solver.h>

#include <string>
#include <iostream>



int main(int argc, char** argv) {
    std::map<std::string, std::string> opts = ParseArgv(argc, argv);
    uint64_t x = std::stoi(opts["target"]);
    Graph g = MakeGraphUsingCsd(x);
    if (Evaluate(g).back() != x) {
        std::cerr << "Internal error\n";
        exit(1);
    }
    std::cout << "OK\n";
    std::cout << "Used nodes " << g.nodes.size() << "\n";
}