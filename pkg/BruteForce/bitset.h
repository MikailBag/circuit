#pragma once

#include <array>
#include <cassert>
#include <concepts>
#include <cstdlib>
#include <stdexcept>
#include <vector>


namespace bf::bs {

template<size_t NDimensionCount>
using Size = std::array<size_t, NDimensionCount>;

template<size_t NDimensionCount>
class BitSet {
    friend class BitSetOps;
private:
    using SizeT = Size<NDimensionCount>;
public:
    explicit BitSet(SizeT size): mInner(Mul(size), false), mSz(size) {
    }

    SizeT size() const {
        return mSz;
    }

    void Fill(bool x) {
        for (size_t i = 0; i < mInner.size(); ++i) {
            mInner[i] = x;
        }
    }

    bool At(std::same_as<size_t> auto ...is) const {
        ValidatePackSize(is...);
        size_t i = ResolveIndex(is...);
        assert(i < mInner.size());
        return mInner[i];
    }

    void Put(bool x, std::same_as<size_t> auto ...is) {
        ValidatePackSize(is...);
        size_t i = ResolveIndex(is...);
        assert(i < mInner.size());
        mInner[i] = x;
    }

    void ApplyPointwiseOr(BitSet const& other) {
        if (mSz != other.mSz) {
            throw std::runtime_error("incompatible sizes");
        }
        for (size_t i = 0; i < mInner.size(); ++i) {
            if (other.mInner[i]) {
                mInner[i] = true;
            }
        }
    }

   

    
private:
    static size_t Mul(SizeT sizes) {
        size_t res = 1;
        for (size_t x : sizes) {
            res *= x;
        }
        return res;
    }
    template<class T>
    struct DepUnit {};
    template<class... Ts>
    void ValidatePackSize([[maybe_unused]] Ts...) const {
        static_assert(NDimensionCount == std::tuple_size_v<std::tuple<DepUnit<Ts>...>>, "Pack size does not match dimension count");
    }
    size_t ResolveIndex(size_t i) const requires (NDimensionCount == 1) {
        return i;
    }
    size_t ResolveIndex(size_t i, size_t j) const requires (NDimensionCount == 2) {
        return i * mSz[1] + j;
    }
    std::vector<bool> mInner;
    SizeT mSz;
};

class BitSetOps {
public:
    BitSetOps() = delete;
    template<size_t N>
    static BitSet<N> SumConv(BitSet<N> const& lhs, BitSet<N> const& rhs) {
        Size<N> sz = lhs.size();
        if (sz != rhs.mSz) {
            throw std::runtime_error("incompatible sizes");
        }
        BitSet out {sz};
        out.Fill(false);
        if constexpr (N == 1) {
            SumConv1(lhs, rhs, out);
        } else if constexpr (N == 2) {
            SumConv2(lhs, rhs, out);
        } else {
            return 0;
        }
        return out;
    }
private:
    static void SumConv1(BitSet<1> const& lhs, BitSet<1> const& rhs, BitSet<1>& out) {
        auto sz = lhs.mSz;
        for (size_t i = 0; i < sz[0]; i++) {
            if (!lhs.At(i)) {
                continue;
            }
            for (size_t j = 0; i + j < sz[0]; j++) {
                if (!rhs.At(j)) {
                    continue;
                }
                out.Put(true, i+j);
            }
            for (size_t j = 0; j <= i; j++) {
                if (!rhs.At(j)) {
                    continue;
                }
                out.Put(true, i-j);
            }
        }
    }
    static void SumConv2(BitSet<2> const& lhs, BitSet<2> const& rhs, BitSet<2>& out) {
        auto sz = lhs.mSz;
        for (size_t i1 = 0; i1 < sz[0]; i1++) {
            for (size_t j1 = 0; j1 < sz[1]; j1++) {
                if (!lhs.At(i1, j1)) {
                    continue;
                }
                for (size_t i2 = 0; i1 + i2 < sz[0]; i2++) {
                    for (size_t j2 = 0; j1 + j2 < sz[1]; j2++) {
                        if (!rhs.At(i2, j2)) {
                            continue;
                        }
                        out.Put(true, i1+i2, j1+j2);
                    }
                }
                for (size_t i2 = 0; i2 <= i1; i2++) {
                    for (size_t j2 = 0; j2 <= j1; j2++) {
                        if (!rhs.At(i2, j2)) {
                            continue;
                        }
                        out.Put(true, i1-i2, j1-j2);
                    }
                }
            }
        }
    }
};
}