#pragma once

#include "conf/parse.h"

#include <string>
#include <vector>

namespace conf {
struct Error {
    std::vector<std::string> path;
    std::string message;
};

ParseException MakeException(std::vector<Error> errors);
}