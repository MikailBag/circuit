#include "argparser/argparser.h"

#include "bruteforce/bruteforce.h"
#include "bruteforce/config.h"
#include "bruteforce/stress.h"

#include "conf/parse.h"

#include "core/topology.h"

#include <iostream>
#include <string>
#include <vector>



int main(int argc, char** argv) {
    argparser::Parser pc;
    pc.AddOption("node-count").DefaultValue("5");
    pc.AddOption("input-count").DefaultValue("1");
    pc.AddOption("filter-config").DefaultValue("()");
    pc.AddOption("left-config").DefaultValue("[alpha:()]");
    pc.AddOption("right-config").DefaultValue("[beta:()]");
    pc.AddOption("launch-config").DefaultValue("(parallel=true)");
    std::map<std::string, std::string> opts = pc.ParseArgv(argc, argv);
    bf::EvalConfig leftConfig;
    bf::EvalConfig rightConfig;
    bf::FilterConfig filterConfig;
    bf::LaunchConfig launchConfig;
    std::cout << "Using configs: " << std::endl;
    std::cout << "Filter: " << opts["filter-config"] << std::endl;
    std::cout << "Left: " << opts["left-config"] << std::endl;
    std::cout << "Right: " << opts["right-config"] << std::endl;
    std::cout << "Launch: " << opts["launch-config"] << std::endl;
     try {
        conf::Parse(opts["filter-config"], filterConfig);
    } catch (conf::ParseException const& ex) {
        std::cerr << "Invalid filter config: " << ex.what() << std::endl;
        return 1;
    }
    try {
        conf::Parse(opts["left-config"], leftConfig);
    } catch (conf::ParseException const& ex) {
        std::cerr << "Invalid left config: " << ex.what() << std::endl;
        return 1;
    }
    try {
        conf::Parse(opts["right-config"], rightConfig);
    } catch (conf::ParseException const& ex) {
        std::cerr << "Invalid right config: " << ex.what() << std::endl;
        return 1;
    }

    try {
        conf::Parse(opts["launch-config"], launchConfig);
    } catch (conf::ParseException const& ex) {
        std::cerr << "Invalid launch config: " << ex.what() << std::endl;
        return 1;
    }

    bf::FindTopologyParams tp;
    tp.inputCount = std::stoi(opts["input-count"]);
    tp.explicitNodeCountLimit = std::stoi(opts["node-count"]);

    bf::EvalParams opLeft {leftConfig, launchConfig};
    bf::EvalParams opRight {rightConfig, launchConfig};
    opLeft.maxExplicitNodeCount = opRight.maxExplicitNodeCount = tp.explicitNodeCountLimit;
    opLeft.inputCount = opRight.inputCount = tp.inputCount;
    
    std::cout << "Input count: " << opLeft.inputCount
              << ", node count: " << tp.explicitNodeCountLimit
              << ", bits " << static_cast<int>(opLeft.config.settings.maxBits)
              << std::endl;
    

    std::vector<bf::Topology> topologies = bf::FindTopologies(tp);

    std::cout << "Topology count: " << topologies.size() << std::endl;

    bf::FilterParams fpLeft {filterConfig};
    bf::FilterParams fpRight {filterConfig};
    fpLeft.inputCount = fpRight.inputCount = tp.inputCount;


    std::vector<bf::Topology> uniqueTopologies = bf::FilterTopologies(fpLeft, topologies);

    std::cout << "Unique topology count: " << uniqueTopologies.size() << std::endl;

    std::optional<bf::stress::Mismatch> failure = bf::stress::Compare(opLeft, opRight, uniqueTopologies);
    if (!failure) {
        std::cout << "No differences found" << std::endl;
        return 0;
    }
    std::cout << "Mismatch at topology #" << failure->topologyIndex << std::endl;
    std::vector<std::string> desc = DescribeTopology(uniqueTopologies[failure->topologyIndex].nodes.span(), tp.inputCount);
    for (std::string const& line : desc) {
        std::cout << "\t" << line << std::endl;
    }
    std::cout << "Output: " << failure->output[0] << std::endl;
    auto boolToString = [](bool x) {
        return x ? "YES" : "NO";
    };
    std::cout << "Left evaluator says " << boolToString(failure->leftResult) << std::endl;
    std::cout << "Right evaluator says " << boolToString(failure->rightResult) << std::endl;
}
