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
    EvalConfig const& config;
    LaunchConfig const& launchConfig;
    size_t maxExplicitNodeCount = 0;
    size_t inputCount = 0;
    std::function<void()> progressListener;

    FindOutputsParams(EvalConfig const& ec, LaunchConfig const& lc) : config(ec), launchConfig(lc) {}
};

}