#pragma once

#include "bruteforce/topology.h"

#include "core/graph.h"

namespace bf::beta {
namespace detail {
class Restore {
public:
    static Graph Do1(Topology const& t, std::span<std::array<int64_t, 1> const> values);
    static Graph Do2(Topology const& t, std::span<std::array<int64_t, 2> const> values);
private:
    Restore() = delete;
};
}
template<size_t N>
Graph Restore(Topology const& t, std::span<std::array<int64_t, N>> values) {
    static_assert(N == 1 || N == 2);
    if constexpr (N == 1) {
        return detail::Restore::Do1(t, values);
    } else {
        return detail::Restore::Do2(t, values);
    }
}
}