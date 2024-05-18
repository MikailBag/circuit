#include "argparser/argparser.h"

#include "bruteforce/bruteforce.h"
#include "bruteforce/config.h"

#include "conf/parse.h"
#include "conf/validation.h"

#include "core/graph.h"

#include <atomic>
#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <memory>


namespace {
std::string Now() {
    return std::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::system_clock::now());
}

struct OutputMode: public conf::Target {
    bool isReachable = false;
    bool isUnreachable = false;
    void Describe(conf::Description& desc) override {
        desc
            .Enum()
            .SimpleVariant("reachable", isReachable)
            .SimpleVariant("unreachable", isUnreachable);
    }

    void Postprocess() override {
        if (!isReachable && !isUnreachable) {
            isReachable = true;
        }
        if (isUnreachable) {
            throw conf::BindingException("todo");
        }
    }
};

struct OutputConfig: public conf::Target {
    OutputMode mode;
    void Describe(conf::Description& desc) override {
        desc
            .Object()
            .ObjField("mode", mode);
    }

    void Postprocess() override {
    }
};

struct Config: public conf::Target {
    bf::FilterConfig filter;
    bf::EvalConfig eval;
    bf::LaunchConfig launch;
    OutputConfig output;

    void Describe(conf::Description& desc) override {
        desc
            .Object()
            .ObjField("filter", filter)
            .ObjField("eval", eval)
            .ObjField("launch", launch)
            .ObjField("output", output);
    }

    void Postprocess() override {
    }
};

class OutputWriter {
public:
    virtual void WriteOutputs(std::span<int64_t const> data, size_t inputCount) = 0;
    ~OutputWriter() = default;
};

class ConsoleOutputWriter : public OutputWriter {
public:
    void WriteOutputs(std::span<int64_t const> data, size_t inputCount) override {
        std::cout << "Possible output count: " << data.size() / inputCount << std::endl;
        for (size_t i = 0; i < data.size(); i += inputCount) {
            std::cout << '(';
            for (size_t j = 0; j < inputCount; j++) {
                if (j > 0) {
                    std::cout << ' ';
                }
                std::cout << data[i+j];
            }
            std::cout << ") ";
        }
        std::cout << std::endl;
    }
};

class FileOutputWriter : public OutputWriter {
public:
    FileOutputWriter(std::ostream& out) : mOut(out) {}

    void WriteOutputs(std::span<int64_t const> data, size_t inputCount) override {
        auto it = std::ostreambuf_iterator<char>{mOut};
        size_t cnt = data.size() / inputCount;
        std::format_to(it, "(row_count={},input_count={})\n", cnt, inputCount);
        for (size_t i = 0; i < data.size(); i += inputCount) {
            *it = '(';
            for (size_t j = 0; j < inputCount; j++) {
                if (j > 0) {
                    *it = ',';
                }
                std::format_to(it, "n{}={}", j, data[i+j]);
            }
            *it = ')';
            *it = '\n';
        }
    }

private:
    std::ostream& mOut;
};

struct FileOutputWriterHolder {
    std::ofstream file;
    std::optional<FileOutputWriter> writer;
};

std::shared_ptr<OutputWriter> MakeFileOutputWriter(std::string_view path) {
    auto sp = std::make_shared<FileOutputWriterHolder>();
    sp->file.exceptions(std::ios::eofbit | std::ios::failbit | std::ios::badbit);
    sp->file.open(std::string{path});
    sp->writer.emplace(sp->file);

    return std::shared_ptr<OutputWriter>{sp, &sp->writer.value()};
}
}

int main(int argc, char** argv) {
    argparser::Parser pc;
    pc.AddOption("node-count").DefaultValue("3");
    pc.AddOption("input-count").DefaultValue("2");
    pc.AddOption("config").DefaultValue("()");
    pc.AddOption("output-path").DefaultValue("cout");
    std::map<std::string, std::string> opts = pc.ParseArgv(argc, argv);
    Config config;
    std::cout << "Using engine config: " << opts["config"] << std::endl;
    try {
        conf::Parse(opts["config"], config);
    } catch (conf::ParseException const& ex) {
        std::cerr << "Invalid config: " << ex.what() << std::endl;
        return 1;
    }
    std::shared_ptr<OutputWriter> outputWriter;
    if (opts["output-path"] == "cout") {
        outputWriter = std::make_shared<ConsoleOutputWriter>();
    } else {
        outputWriter = MakeFileOutputWriter(opts["output-path"]);
    }

    size_t maxNodeCount = std::stoi(opts["node-count"]);
    for (size_t nodeCount = 1; nodeCount <= maxNodeCount; nodeCount++) {
        bf::FindTopologyParams tp;
        tp.inputCount = std::stoi(opts["input-count"]);
        tp.explicitNodeCountLimit = nodeCount;

        bf::EvalParams op {config.eval, config.launch};
        op.maxExplicitNodeCount = nodeCount;
        op.inputCount = tp.inputCount;
        std::cout << "Input count: " << op.inputCount
                  << ", node count: " << nodeCount
                  << std::endl;

        std::vector<bf::Topology> topologies = bf::FindTopologies(tp);

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

        std::vector<int64_t> outputs = bf::EvalTopologies(op, uniqueTopologies);
        assert(outputs.size() % op.inputCount == 0);
        outputWriter->WriteOutputs({outputs.data(), outputs.size()}, op.inputCount);
    }
}
