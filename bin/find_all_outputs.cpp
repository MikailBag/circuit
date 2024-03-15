#include "argparser/argparser.h"
#include "bruteforce/bruteforce.h"
#include "graph.h"

#include <string>
#include <iostream>



int main(int argc, char** argv) {
    argparser::ParserConfig pc;
    pc.AddOption("node-count").DefaultValue("3");
    pc.AddOption("bits").DefaultValue("3");
    std::map<std::string, std::string> opts = argparser::ParseArgv(pc, argc, argv);
    bruteforce::FindTopologyParams tp;
    tp.explicitNodeCountLimit = std::stoi(opts["node-count"]);

    bruteforce::FindOutputsParams op;
    op.bits = std::stoi(opts["bits"]);
    std::cout << "Input count: " << bruteforce::kInCount
              << ", node count: " << tp.explicitNodeCountLimit
              << ", bits " << static_cast<int>(op.bits)
              << std::endl;

    std::vector<bruteforce::Topology> topologies = bruteforce::FindAllTopologies(tp);

    std::cout << "Topology count: " << topologies.size() << std::endl;

    std::vector<bruteforce::Topology> uniqueTopologies = bruteforce::FilterTopologies(topologies);

    std::cout << "Unique topology count: " << uniqueTopologies.size() << std::endl;

    std::vector<uint64_t> outputs = bruteforce::FindAllOutputs(op, uniqueTopologies);

    std::cout << "Possible output count: " << outputs.size();
}
