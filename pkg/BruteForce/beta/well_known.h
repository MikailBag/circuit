#pragma once

#include <cstdlib>
namespace bf::beta {
namespace detail {
class WellKnown {
public:
    static ssize_t IsWellKnown(int64_t x, int64_t y);
    static size_t MaxIndex();
};
}

inline ssize_t IsWellKnown(int64_t x, int64_t y) {
    return detail::WellKnown::IsWellKnown(x, y);
}
inline size_t MaxIndex() {
    return detail::WellKnown::MaxIndex();
}
}