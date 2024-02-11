#include "argparser.h"

int main(int argc, char** argv) {
    argparser::ParserConfig pc;
    std::map<std::string, std::string> opts = ParseArgv(pc, argc, argv);
}