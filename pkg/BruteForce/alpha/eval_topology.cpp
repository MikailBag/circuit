#include "eval_topology.h"

#include "bitset.h"

#include "log/log.h"

#include <cassert>


static Logger L = GetLogger("bf.alpha.topeval");

namespace bf::alpha {
void PushShifts1(bs::BitSet<1>& v) {
    bs::Size<1> n = v.size();
    for (size_t i = 0; 2*i < n[0]; i++) {
        if (v.At(i)) {
            L().AttrU64("src", i).AttrU64("dst", 2*i).Log("Pushing");
            v.Put(true, 2*i);
        }
    }
}

void PushShifts2(bs::BitSet<2>& v) {
    bs::Size<2> n = v.size();
    for (size_t i = 0; 2*i < n[0]; i++) {
        for (size_t j = 0; 2*j < n[1]; j++) {
            if (v.At(i, j)) {
                L().AttrU64("src0", i).AttrU64("src1", j).AttrU64("dst0", 2*i).AttrU64("dst1", 2*j).Log("Pushing");
                v.Put(true, 2*i, 2*j);
            }
        }
    }
}



template<size_t N>
void PushShifts(bs::BitSet<N>& v) {
    static_assert(N == 1 || N == 2, "TODO");
    if constexpr (N == 1) {
        PushShifts1(v);
    } else if (N == 2) {
        PushShifts2(v);
    }
}


template<size_t N>
void FindTopologyOutputs(Topology const& t, size_t inputs, uint8_t bits, bs::BitSet<N>& ans, std::vector<bs::BitSet<N>> bufs) {
    assert(bufs.size() >= t.size() + inputs);
    size_t maxVal = 1 << static_cast<size_t>(bits);
    for (size_t i = 0; i < t.size(); i++) {
        bufs[i+inputs].Fill(false);
    }
    for (size_t i = 0; i < t.size(); i++) {
        size_t lhsIdx = t[i].links[0];
        size_t rhsIdx = t[i].links[1];
        L().AttrU64("lhs", lhsIdx).AttrU64("rhs", rhsIdx).Log("Processing node");
        bufs[i+inputs] = bs::BitSetOps::SumConv(bufs[lhsIdx], bufs[rhsIdx]);
        PushShifts(bufs[i+inputs]);
    }
    for (size_t i = 0; i < t.size() + inputs; i++) {
        for (size_t j = 0; j <= maxVal; j++) {
            ans.ApplyPointwiseOr(bufs[i]);
        }
    }
}
template void PushShifts<1>(bs::BitSet<1>& v);
template void PushShifts<2>(bs::BitSet<2>& v);


template void FindTopologyOutputs<1>(Topology const& t, size_t inputs, uint8_t bits, bs::BitSet<1>& ans, std::vector<bs::BitSet<1>> bufs);
template void FindTopologyOutputs<2>(Topology const& t, size_t inputs, uint8_t bits, bs::BitSet<2>& ans, std::vector<bs::BitSet<2>> bufs);
}