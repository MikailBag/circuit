#include "signed_pack.h"

namespace bf {

uint64_t PackSigned(int64_t x) {
    if (x >= 0) {
        return static_cast<uint64_t>(x) * 2;
    }
    return static_cast<uint64_t>(-x) * 2 - 1;
}

int64_t UnpackSigned(uint64_t x) {
    if (x % 2 == 0) {
        return static_cast<int64_t>(x / 2);
    } else {
        return -1 - static_cast<int64_t>(x / 2); 
    }
}

}