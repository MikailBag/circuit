#include "bruteforce/config.h"

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
}