#pragma once

#include "config.h"

#include <functional>


namespace bf {

struct FindTopologyParams {
    size_t inputCount = 0;
    size_t explicitNodeCountLimit = 0;
};

struct FilterParams {
    size_t inputCount = 0;
    FilterConfig const& config;

    explicit FilterParams(FilterConfig const& c) : config(c) {}
};

struct FindOutputsParams {
    OutputConfig const& config;
    LaunchConfig const& launchConfig;
    uint8_t maxBits = 0;
    size_t maxExplicitNodeCount = 0;
    size_t inputCount = 0;
    std::function<void()> progressListener;

    FindOutputsParams(OutputConfig const& c, LaunchConfig const& lc) : config(c), launchConfig(lc) {}
};

}