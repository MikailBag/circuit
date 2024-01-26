#pragma once
#include "graph.h"
#include <cstddef>


class GraphBuilder {
public:
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
        AddNode(left, right, true);
        return std::move(*this);
    }

    GraphBuilder Sub(LinkB left, LinkB right) && {
        AddNode(left, right, false);
        return std::move(*this);
    }
    size_t NodeCount() const {
        return mG.nodes.size() + 1;
    }

    Graph Build() && {
        return std::move(mG);
    }

private:
    void AddNode(LinkB left, LinkB right, bool add);
    Graph mG;
};

GraphBuilder::LinkB ByIndex(size_t idx);