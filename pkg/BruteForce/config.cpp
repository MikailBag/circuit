#include "bruteforce/config.h"

#include "conf/validation.h"

namespace bf {
void LaunchConfig::Postprocess() {
    if (parallel) {
        if (threadCount == 0) {
            throw conf::BindingException("thread_count must be non-zero in parallel mode");
        }
    }
}

void ShardingConfig::RangeSharding::Postprocess() {
    if (rangeStart >= totalPartCount) {
        throw conf::BindingException("range_start must be in [0, range_size)");
    }
    if (rangeSize > totalPartCount - rangeStart) {
        throw conf::BindingException("range should not exceed total part count");
    }
}

void EvalEngineConfig::Beta::Postprocess() {
    if (skipValidation) {
        throw conf::BindingException("TODO");
    }
}

void EvalEngineConfig::Beta::PrintTopology::Postprocess() {
    if (!enabled) {
        if (x != 0 || y != 0) {
            throw conf::BindingException("x or y can not be set when enabled is false");
        }
    }
}
}