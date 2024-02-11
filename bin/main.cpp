#include <iostream>
#include "csd.h"
//#include "graph.h"
//#include "graph_builder.h"
int main() {
    /*Graph g = GraphBuilder()
        .Add(ByIndex(0).Shifted(0), ByIndex(0).Shifted(3)) // 1 + 8 = 9
        .Sub(ByIndex(1).Shifted(0), ByIndex(0).Shifted(2)) // 9 - 4 = 5
        .Build();
    std::vector<uint32_t> outs = Evaluate(g);
    for (uint32_t x : outs) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
    */
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