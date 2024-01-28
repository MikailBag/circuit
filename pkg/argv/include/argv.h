#pragma once
#include <string>

#include <map>

std::map<std::string, std::string> ParseArgv(size_t argc, char const* const* argv);