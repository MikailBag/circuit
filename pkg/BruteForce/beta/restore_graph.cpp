#include "beta/restore_graph.h"

#include "bruteforce/topology.h"

#include "core/graph_builder.h"

#include <array>
#include <span>

namespace bf::beta {
namespace {
struct SignedShift {
    bool negate = false;
    uint8_t shift = 0;
};

template<size_t N>
bool CheckShifts(std::array<int64_t, N> lhs, std::array<int64_t, N> rhs, std::array<int64_t, N> res, SignedShift leftShift, SignedShift rightShift) {
    for (size_t i = 0; i < N; i++) {
        int64_t x = 0;
        if (leftShift.negate) {
            x -= lhs[i] << leftShift.shift;
        } else {
            x += lhs[i] << leftShift.shift;
        }
        if (rightShift.negate) {
            x -= rhs[i] << rightShift.shift;
        } else {
            x += rhs[i] << rightShift.shift;
        }
        if (x != res[i]) {
            return false;
        }
    }
    return true;
}

template<size_t N>
std::pair<SignedShift, SignedShift> RestoreShifts([[maybe_unused]] std::array<int64_t, N> lhs, [[maybe_unused]] std::array<int64_t, N> rhs, [[maybe_unused]] std::array<int64_t, N> res) {
    for (uint8_t shiftMax = 0;; shiftMax++) {
        assert(shiftMax < 20);
        for (uint8_t shiftOther = 0; shiftOther <= shiftMax; shiftOther++) {
            for (uint i = 0; i < 3; i++) {
                for (uint j = 0; j < 2; j++) {
                    SignedShift sl = {.negate = (i % 2 == 1), .shift = shiftMax};
                    SignedShift sr = {.negate = (i / 2 == 1), .shift = shiftOther};
                    if (j == 1) {
                       std::swap(sl.shift, sr.shift);
                    }

                    if (CheckShifts(lhs, rhs, res, sl, sr)) {
                        return {sl, sr};
                    }
                }
            }
        }
    }
    assert(false);
}

template<size_t N>
Graph RestoreImpl(Topology const& t, std::span<std::array<int64_t, N> const> values) {
    GraphBuilder b {N};
    size_t sz = t.nodes.size();
    assert(values.size() == sz + N);
    for (size_t i = 0; i < sz; i++) {
        size_t leftIdx = t.nodes[i].links[0];
        size_t rightIdx = t.nodes[i].links[1];
        auto [leftShift, rightShift] = RestoreShifts(values[leftIdx], values[rightIdx], values[i + N]);
        if (leftShift.negate && rightShift.negate) {
            assert(false);
        }
        GraphBuilder::LinkB leftLink = ByIndex(leftIdx).Shifted(leftShift.shift);
        GraphBuilder::LinkB rightLink = ByIndex(rightIdx).Shifted(rightShift.shift);
        if (!leftShift.negate && !rightShift.negate) {
            b = std::move(b).Add(leftLink, rightLink);
        } else if (leftShift.negate) {
            b = std::move(b).Sub(rightLink, leftLink);
        } else {
            b = std::move(b).Sub(leftLink, rightLink);
        }
    }
    return std::move(b).Build();
}
}

Graph detail::Restore::Do1(Topology const& t, std::span<std::array<int64_t, 1> const> values) {
    return RestoreImpl(t, values);
}

Graph detail::Restore::Do2(Topology const& t, std::span<std::array<int64_t, 2> const> values) {
    return RestoreImpl(t, values);
}

}