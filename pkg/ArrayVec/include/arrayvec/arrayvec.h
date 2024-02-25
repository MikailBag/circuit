#pragma once

#include <array>
#include <cassert>
#include <cstddef>
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

        T* Ptr() {
            void* bPtr = Place();
            return std::launder(reinterpret_cast<T*>(bPtr));
        }

        T const* Ptr( ) const {
            void* bPtr = Place();
            return std::launder(reinterpret_cast<T*>(bPtr));
        }
    };
    std::array<Slot, N> mSlots;
    std::size_t mSize;
public:
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
private:
};

}