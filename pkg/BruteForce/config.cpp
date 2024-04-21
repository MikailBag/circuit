#include "bruteforce/config.h"

namespace bf {
void LaunchConfig::Postprocess() {
    if (parallel) {
        if (threadCount == 0) {
            throw conf::BindingException("threadCount must be non-zero in parallel mode");
        }
    }
}
}