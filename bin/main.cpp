#include "csd.h"

#include <iostream>


int main() {
    for (int i = 0; i <= 1000; i++) {
        for (int j = 0; j <= 1000; j++) {
            CanonicalSignedDigitRepresentation ci = EncodeCsd(i);
            CanonicalSignedDigitRepresentation cj = EncodeCsd(j);
            CanonicalSignedDigitRepresentation csum = EncodeCsd(i+j);
            if (ci.PopCount() + cj.PopCount() < csum.PopCount()) {
                std::cout << i << ' ' << j << std::endl;
                return 1;
            }
        }
    }
    std::cout << "OK" << std::endl;
    return 0;
}