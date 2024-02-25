#include "argparser.h"
#include "find_all_outputs.h"
#include "graph.h"

#include <string>
#include <iostream>



int main(int argc, char** argv) {
    argparser::ParserConfig pc;
    pc.AddOption("node-count").DefaultValue("3");
    pc.AddOption("bits").DefaultValue("3");
    std::map<std::string, std::string> opts = argparser::ParseArgv(pc, argc, argv);
    find_all_outputs::FindTopologyParams tp;
    tp.explicitNodeCountLimit = std::stoi(opts["node-count"]);

    find_all_outputs::FindOutputsParams op;
    op.bits = std::stoi(opts["bits"]);
    std::cout << "Input count: " << find_all_outputs::kInCount
              << ", node count: " << tp.explicitNodeCountLimit
              << ", bits " << static_cast<int>(op.bits)
              << std::endl;

    std::vector<find_all_outputs::Topology> topologies = find_all_outputs::FindAllTopologies(tp);

    std::cout << "Topology count: " << topologies.size() << std::endl;

    std::vector<find_all_outputs::Topology> uniqueTopologies = find_all_outputs::FilterTopologies(topologies);

    std::cout << "Unique topology count: " << uniqueTopologies.size() << std::endl;

    std::vector<uint64_t> outputs = find_all_outputs::FindAllOutputs(op, uniqueTopologies);

    std::cout << "Possible output count: " << outputs.size();
}
