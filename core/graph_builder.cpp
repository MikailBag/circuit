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

void GraphBuilder::AddNodeCommon(LinkB const& left, LinkB const& right) {
    if (!left.mShiftSet || !right.mShiftSet) {
        throw std::logic_error("Shifted() was not called");
    }
    if (left.mIdx > mG.nodes.size()) {
        throw std::invalid_argument("left link has too big index");
    }
    if (right.mIdx > mG.nodes.size()) {
        throw std::invalid_argument("right link has too big index");
    }
}

void GraphBuilder::AddNode(LinkB left, LinkB right, bool add) {
    GraphNode n;
    n.links[0] = Link::OfAdd(left.mIdx, left.mShift);
    n.links[1] = add ? Link::OfAdd(right.mIdx, right.mShift) : Link::OfSub(right.mIdx, right.mShift);
    mG.nodes.push_back(std::move(n));
}

GraphBuilder::LinkB ByIndex(size_t idx) {
    GraphBuilder::LinkB l;
    l.mIdx = idx;
    l.mShiftSet = false;
    return l;
}