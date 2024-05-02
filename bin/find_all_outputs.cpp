#include "argparser/argparser.h"

#include "bruteforce/bruteforce.h"
#include "bruteforce/config.h"

#include "conf/conf.h"

#include "graph.h"

#include <atomic>
#include <chrono>
#include <format>
#include <string>
#include <iostream>
#include <memory>


namespace {
std::string Now() {
    return std::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::system_clock::now());
}


struct Config: public conf::Target {
    bf::FilterConfig filter;
    bf::EvalConfig eval;
    bf::LaunchConfig launch;

    void Describe(conf::Description& desc) override {
        desc.IsObject();
        desc.ObjField("filter", filter);
        desc.ObjField("eval", eval);
        desc.ObjField("launch", launch);
    }

    void Postprocess() override {
    }
};
}




int main(int argc, char** argv) {
    argparser::Parser pc;
    pc.AddOption("node-count").DefaultValue("3");
    pc.AddOption("bits").DefaultValue("3");
    pc.AddOption("input-count").DefaultValue("2");
    pc.AddOption("config").DefaultValue("()");
    std::map<std::string, std::string> opts = pc.ParseArgv(argc, argv);
    Config config;
    std::cout << "Using engine config: " << opts["config"] << std::endl;
    try {
        conf::Parse(opts["config"], config);
    } catch (conf::ParseException const& ex) {
        std::cerr << "Invalid config: " << ex.what() << std::endl;
        return 1;
    }

    size_t maxNodeCount = std::stoi(opts["node-count"]);
    for (size_t nodeCount = 1; nodeCount <= maxNodeCount; nodeCount++) {
        bf::FindTopologyParams tp;
        tp.inputCount = std::stoi(opts["input-count"]);
        tp.explicitNodeCountLimit = nodeCount;

        bf::FindOutputsParams op {config.eval, config.launch};
        op.maxBits = std::stoi(opts["bits"]);
        op.maxExplicitNodeCount = nodeCount;
        op.inputCount = tp.inputCount;
        std::cout << "Input count: " << op.inputCount
                  << ", node count: " << nodeCount
                  << ", bits " << static_cast<int>(op.maxBits)
                  << std::endl;

        std::vector<bf::Topology> topologies = bf::FindAllTopologies(tp);

        std::cout << "Topology count: " << topologies.size() << std::endl;

        bf::FilterParams fp {config.filter};
        fp.inputCount = tp.inputCount;


        std::vector<bf::Topology> uniqueTopologies = bf::FilterTopologies(fp, topologies);

        std::cout << "Unique topology count: " << uniqueTopologies.size() << std::endl;

        std::atomic<size_t> finishedTasks = 0;

        op.progressListener = [uniqueTopologies = std::cref(uniqueTopologies), finishedTasks=&finishedTasks]() {
            size_t finished = finishedTasks->fetch_add(1, std::memory_order::relaxed);
            if (finished % 100 == 0) {
                std::string now = Now();
                std::cout << "[" << now << "] " << "Progress: " << finished << " / " << uniqueTopologies.get().size() << std::endl;
            }
        };

        std::vector<uint64_t> outputs = bf::FindAllOutputs(op, uniqueTopologies);

 
        std::cout << "Possible output count: " << outputs.size() / op.inputCount << std::endl;
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
}
