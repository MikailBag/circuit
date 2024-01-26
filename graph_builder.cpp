#include "graph_builder.h"
#include <stdexcept>

GraphBuilder::LinkB& GraphBuilder::LinkB::Shifted(size_t shift) && {
    if (mShiftSet) {
        throw std::logic_error("Shifted() called twice");
    }
    mShiftSet = true;
    mShift = shift;
    return *this;
}


void GraphBuilder::AddNode(LinkB left, LinkB right, bool add) {
    if (!left.mShiftSet || !right.mShiftSet) {
        throw std::logic_error("Shifted() was not called");
    }
    if (left.mIdx > mG.nodes.size()) {
        throw std::invalid_argument("left link has too big index");
    }
    if (right.mIdx > mG.nodes.size()) {
        throw std::invalid_argument("right link has too big index");
    }
    GraphNode n;
    n.links[0].index = left.mIdx;
    n.links[0].shiftWithSign = left.mShift;
    n.links[1].index = right.mIdx;
    n.links[1].shiftWithSign = right.mShift;
    if (!add) {
        n.links[1].shiftWithSign |= kShiftSignBit;
    }
    mG.nodes.push_back(n);
}

GraphBuilder::LinkB ByIndex(size_t idx) {
    GraphBuilder::LinkB l;
    l.mIdx = idx;
    l.mShiftSet = false;
    return l;
}