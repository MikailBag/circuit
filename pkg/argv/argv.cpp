#include "argv.h"

#include <cstdlib>
#include <iostream>
#include <string_view>

static bool StartsWith(std::string_view s, std::string_view prefix) {
    if (s.size() < prefix.size()) {
        return false;
    }
    for (size_t i = 0; i < prefix.size(); ++i) {
        if (s[i] != prefix[i]) {
            return false;
        }
    }
    return true;
}

std::map<std::string, std::string> ParseArgv(size_t argc, char const* const* argv) {
    if (argc <= 1) {
        return {};
    }
    std::map<std::string, std::string> res;
    for (size_t i = 1; i < argc; i += 2) {
        std::string_view key = argv[i];
        if (i+1 == argc) {
            std::cerr << "no value for key\n";
            exit(1);
        }
        if (!StartsWith(key, "--") || key.size() == 2) {
            std::cerr << "invalid option " << key << '\n';
            exit(1);
        }
        res[std::string(key.substr(2))] = argv[i+1];
    }
    return res;
}