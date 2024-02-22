#include "argparser.h"
#include "find_all_outputs.h"
#include "graph.h"

#include <string>
#include <iostream>



int main(int argc, char** argv) {
    argparser::ParserConfig pc;
    pc.AddOption("node-count").DefaultValue("3");
    pc.AddOption("bits").DefaultValue("5");
    std::map<std::string, std::string> opts = ParseArgv(pc, argc, argv);
    Params p;
    p.explicitNodeCountLimit = std::stoi(opts["node-count"]);
    int bits = std::stoi(opts["bits"]);
    std::cout << "Input count: " << kInCount << ", node count: " << p.explicitNodeCountLimit << ", bits " << bits << std::endl;

    std::vector<Graph> ans;
    FindAllTopologies(p, ans);

    std::cout << "Topology count: " << ans.size() << std::endl;
}
