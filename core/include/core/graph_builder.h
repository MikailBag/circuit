#pragma once
#include "core/graph.h"
#include <cstddef>


class GraphBuilder {
public:
    explicit GraphBuilder(size_t inputCount);

    class LinkB {
        friend class GraphBuilder;
        friend LinkB ByIndex(size_t);
    public:
        LinkB& Shifted(size_t shift) &&;
    private:
        size_t mIdx;
        uint8_t mShift;
        bool mShiftSet;
    };
    GraphBuilder Add(LinkB left, LinkB right) && {
        AddNodeCommon(left, right);
        mG.nodes.emplace_back(Link::OfAdd(left.mIdx, left.mShift), Link::OfAdd(right.mIdx, right.mShift));
        return std::move(*this);
    }

    GraphBuilder Sub(LinkB left, LinkB right) && {
        AddNodeCommon(left, right);
        mG.nodes.emplace_back(Link::OfAdd(left.mIdx, left.mShift), Link::OfSub(right.mIdx, right.mShift));
        return std::move(*this);
    }
    size_t NodeCount() const {
        return mG.nodes.size() + 1;
    }

    Graph Build() && {
        return std::move(mG);
    }

private:
    void AddNodeCommon(LinkB const& left, LinkB const& right);
    void AddNode(LinkB left, LinkB right, bool add);
    size_t mInputCount;
    Graph mG;
};

GraphBuilder::LinkB ByIndex(size_t idx);