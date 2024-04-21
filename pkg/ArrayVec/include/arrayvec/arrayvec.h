#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <span>
#include <utility>

namespace arrayvec {
namespace detail {

inline constexpr bool kEnableBoundChecks = true;

}

template<class T, size_t N>
class ArrayVec {
private:
    struct Slot {
        alignas(alignof(T))
        char buf[sizeof(T)];

        void* Place() {
            return buf;
        }

        void const* Place() const {
            return buf;
        }

        T* Ptr() {
            void* bPtr = Place();
            return std::launder(reinterpret_cast<T*>(bPtr));
        }

        T const* Ptr() const {
            void const* bPtr = Place();
            return std::launder(reinterpret_cast<T const*>(bPtr));
        }

        void Destroy() {
            Ptr()->~T();
        }
    };
    std::array<Slot, N> mSlots;
    std::size_t mSize = 0;
public:
    static_assert(std::is_trivially_destructible_v<T>);
    ArrayVec() {}
    ArrayVec(ArrayVec const& that) noexcept {
        for (size_t i = 0; i < that.mSize; ++i) {
            new (mSlots[i].Place()) T(that[i]);
        }
        mSize = that.mSize;
    }
    ArrayVec(ArrayVec&& that) noexcept {
        for (size_t i = 0; i < that.mSize; ++i) {
            new (mSlots[i].Place()) T(std::move(that[i]));
        }
        mSize = that.mSize;
    }

    void push_back(T&& value) {
        if (detail::kEnableBoundChecks) {
            assert(mSize < N);
        }
        new(mSlots[mSize].Place()) T(std::move(value));
        mSize++;
    }

    void push_back(T const& value) {
        push_back(T(value));
    }

    size_t size() const {
        return mSize;
    }

    T& operator[](size_t idx) {
        if (detail::kEnableBoundChecks) {
            assert(idx < mSize);
        }
        return *mSlots[idx].Ptr();
    }

    T const& operator[](size_t idx) const {
        if (detail::kEnableBoundChecks) {
            assert(idx < mSize);
        }
        return *mSlots[idx].Ptr();
    }

    std::span<T const> span() const {
        return std::span{std::launder(reinterpret_cast<T const*>(mSlots.data())), mSize};
    }

    std::span<T> span() {
        return std::span{std::launder(reinterpret_cast<T*>(mSlots.data())), mSize};
    }

    ~ArrayVec() {
        for (size_t i = 0; i < mSize; ++i) {
            mSlots[i].Destroy();
        }
    }
};

}