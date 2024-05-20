#include "core/graph.h"

#include <stdexcept>
#include <string>
#include <iostream>
#include <cassert>

static void SafeShiftErr(uint64_t x, uint64_t shift) {
    throw std::overflow_error("overflow when evaluating " + std::to_string(x) + " << " + std::to_string(shift));
}

static uint64_t SafeShift(uint64_t x, uint64_t shift) {
    if (shift >= 64) {
        SafeShiftErr(x, shift);
    }
    if (shift == 0) {
        return x;
    }
    // x * (1 << shift) < (1 << 32)
    // x < (1 << (32-shift))
    uint32_t bound = 1;
    bound <<= 64 - shift;
    if (x >= bound) {
        SafeShiftErr(x, shift);
    }
    return (x << shift);
}

static uint64_t SafeAdd(uint64_t x, uint64_t y) {
    if (x > UINT32_MAX - y) {
        throw std::overflow_error("overflow when evaluating " + std::to_string(x) + " + " + std::to_string(y));
    }
    return x + y;
}

static uint64_t SubAbs(uint64_t x, uint64_t y) {
    if (x >= y) {
        return x - y;
    }
    return y - x;
}

bool Link::IsNegated() const {
    return shiftWithSign & kShiftSignBit;
}

Link Link::OfAdd(size_t i, uint8_t shift) {
    Link l;
    l.index = i;
    l.shiftWithSign = shift;
    return l;
}

Link Link::OfSub(size_t i, uint8_t shift) {
    Link l;
    l.index = i;
    l.shiftWithSign = shift | kShiftSignBit;
    return l;
}

static constexpr uint8_t kRestoreShiftMark = 0x7f;

uint8_t Link::Shift() const {
    return shiftWithSign & kRestoreShiftMark;
}

std::vector<uint64_t> Evaluate1D(Graph const& g) {
    std::vector<uint64_t> out;
    out.reserve(g.nodes.size() + 1);
    out.push_back(1);
    for (GraphNode const& n : g.nodes) {
        if (n.links[0].IsNegated()) {
            throw std::invalid_argument("first link must not be negated");
        }
        auto i = static_cast<size_t>(n.links[0].index);
        auto j = static_cast<size_t>(n.links[1].index);
        if (i >= out.size() || j >= out.size()) {
            throw std::invalid_argument("invalid index");
        }
        uint64_t v0 = SafeShift(out[i], n.links[0].Shift());
        uint64_t v1 = SafeShift(out[j], n.links[1].Shift());
        uint64_t res;
        if (n.links[1].IsNegated()) {
            res = SubAbs(v0, v1);
        } else {
            res = SafeAdd(v0, v1);
        }
        out.push_back(res);
    }
    return out;
}

std::vector<std::pair<int64_t, int64_t>> Evaluate2D(Graph const& g) {
    std::vector<std::pair<int64_t, int64_t>> out;
    out.reserve(g.nodes.size() + 2);
    out.push_back({1, 0});
    out.push_back({0, 1});
    for (GraphNode const& n : g.nodes) {
        auto i = static_cast<size_t>(n.links[0].index);
        auto j = static_cast<size_t>(n.links[1].index);
        if (i >= out.size() || j >= out.size()) {
            throw std::invalid_argument("invalid index");
        }
        int64_t v0x = (out[i].first << n.links[0].Shift());
        int64_t v0y = (out[i].second << n.links[0].Shift());
        int64_t v1x = (out[j].first << n.links[1].Shift());
        int64_t v1y = (out[j].second << n.links[1].Shift());
        std::pair<int64_t, int64_t> res;
        if (n.links[1].IsNegated()) {
            res = {SubAbs(v0x, v1x), SubAbs(v0y, v1y)};
        } else {
            res = {SafeAdd(v0x, v1x), SafeAdd(v0y, v1y)};
        }
        out.push_back(res);
    }
    return out;
}


std::vector<std::string> Describe(Graph const& g) {
    std::vector<std::string> out;
    out.push_back("[0] = 1 (input)");
    size_t counter = 0;
    for (GraphNode const& n : g.nodes) {
        ++counter;
        auto i = static_cast<size_t>(n.links[0].index);
        auto j = static_cast<size_t>(n.links[1].index);
        std::string res = "[";
        res += std::to_string(counter);
        res += "] = [";
        res += std::to_string(i);
        res += "] << ";
        res += std::to_string(n.links[0].Shift());
        if (n.links[1].IsNegated()) {
            res += " - [";
        } else {
            res += " + [";
        }
        res += std::to_string(j);
        res += "] << ";
        res += std::to_string(n.links[1].Shift());
        out.push_back(std::move(res));
    }
    return out;
}

Graph Graph::Merge(size_t inputCount, Graph lhs, Graph const& rhs) {
    assert(inputCount == 1); // TODO
    assert(lhs.IsValid(inputCount));
    assert(rhs.IsValid(inputCount));
    size_t lhsSz = lhs.nodes.size();
    // 0 - ONE
    // [1 .. lhsSz] - left nodes
    // [lhsSz + 1 .. lhsSz + 1 + rhsSz) - right nodes
    auto remapIndex = [lhsSz](size_t i) -> size_t {
        if (i == 0) {
            return 0;
        }
        return i + lhsSz;
    };
    for (size_t i = 0; i < rhs.nodes.size(); ++i) {
        GraphNode gn = rhs.nodes[i];
        gn.links[0].index = remapIndex(gn.links[0].index);
        gn.links[1].index = remapIndex(gn.links[1].index);
        lhs.nodes.push_back(gn);
    }
    assert(lhs.IsValid(inputCount));
    return lhs;
}

bool Graph::IsValid(size_t inputCount) const {
    for (size_t i = 0; i < nodes.size(); ++i) {
        size_t threshold = i + inputCount;
        if (nodes[i].links[0].index >= threshold || nodes[i].links[1].index >= threshold) {
            return false;
        }
    }
    return true;
}