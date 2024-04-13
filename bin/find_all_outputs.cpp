#include "argparser/argparser.h"

#include "bruteforce/bruteforce.h"
#include "bruteforce/config.h"

#include "conf/conf.h"

#include "graph.h"

#include <string>
#include <iostream>
#include <memory>



int main(int argc, char** argv) {
    argparser::Parser pc;
    pc.AddOption("node-count").DefaultValue("3");
    pc.AddOption("bits").DefaultValue("3");
    pc.AddOption("input-count").DefaultValue("2");
    pc.AddOption("config").DefaultValue("()");
    std::map<std::string, std::string> opts = pc.ParseArgv(argc, argv);
    bf::Config config;
    try {
        std::cout << "Using engine config: " << opts["config"] << std::endl;
        conf::Parse(opts["config"], config);
    } catch (conf::ParseException const& ex) {
        std::cerr << "Invalid config: " << ex.what() << std::endl;
        return 1;
    }

    bf::FindTopologyParams tp;
    tp.inputCount = std::stoi(opts["input-count"]);
    tp.explicitNodeCountLimit = std::stoi(opts["node-count"]);

    bf::FindOutputsParams op;
    op.maxBits = std::stoi(opts["bits"]);
    op.maxExplicitNodeCount = tp.explicitNodeCountLimit;
    op.inputCount = tp.inputCount;
    std::cout << "Input count: " << op.inputCount
              << ", node count: " << tp.explicitNodeCountLimit
              << ", bits " << static_cast<int>(op.maxBits)
              << std::endl;

    std::vector<bf::Topology> topologies = bf::FindAllTopologies(tp);

    std::cout << "Topology count: " << topologies.size() << std::endl;

    bf::FilterParams fp {.config = config};
    fp.inputCount = tp.inputCount;


    std::vector<bf::Topology> uniqueTopologies = bf::FilterTopologies(fp, topologies);

    std::cout << "Unique topology count: " << uniqueTopologies.size() << std::endl;

    op.progressListener = [uniqueTopologies = std::cref(uniqueTopologies)](bf::ProgressEvent const& ev) {
        if (ev.finished % 100 == 0) {
            std::cout << "Progress: " << ev.finished << " / " << uniqueTopologies.get().size() << std::endl;
        }
    };

    std::vector<uint64_t> outputs = bf::FindAllOutputs(op, uniqueTopologies);

 
    std::cout << "Impossible output count: " << outputs.size() / op.inputCount << std::endl;
    assert(outputs.size() % op.inputCount == 0);
    for (size_t i = 0; i < outputs.size(); i += op.inputCount) {
        std::cout << '(';
        for (size_t j = 0; j < op.inputCount; j++) {
            if (j > 0) {
                std::cout << ' ';
            }
            std::cout << outputs[i+j];
        }
        std::cout << ") ";
    }
    std::cout << std::endl;
}
