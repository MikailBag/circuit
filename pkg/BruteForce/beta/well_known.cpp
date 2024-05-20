#include "beta/well_known.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace {

std::vector<std::pair<int64_t, int64_t>> WELL_KNOWN = []{
    std::vector<std::pair<int64_t, int64_t>> t;
    t.emplace_back(255, -18);
    t.emplace_back(127, -12);
    t.emplace_back(63, -8);
    t.emplace_back(63, -9);
    t.emplace_back(63, -11);
    t.emplace_back(249, -56);
    t.emplace_back(124, -31);
    t.emplace_back(123, -35);
    t.emplace_back(61, -18);
    t.emplace_back(30, -11);
    t.emplace_back(59, -24);
    t.emplace_back(29, -13);
    t.emplace_back(231, -110);
    t.emplace_back(114, -57);
    t.emplace_back(113, -60);
    t.emplace_back(219, -132);
    t.emplace_back(27, -17);
    t.emplace_back(99, -80);
    std::sort(t.begin(), t.end());
    return t;
}();


}

ssize_t IsWellKnown(int64_t x, int64_t y) {
    auto it = std::lower_bound(WELL_KNOWN.begin(), WELL_KNOWN.end(), x, [](auto x, auto y) {
        return x.first < y;
    });
    if (it == WELL_KNOWN.end()) {
        return -1;
    }
    if (*it != std::pair{x, y}) {
        return -1;
    }
    return it - WELL_KNOWN.begin();
}