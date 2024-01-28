#include <iostream>
#include "graph.h"
#include "graph_builder.h"
int main() {
    Graph g = GraphBuilder()
        .Add(ByIndex(0).Shifted(0), ByIndex(0).Shifted(3)) // 1 + 8 = 9
        .Sub(ByIndex(1).Shifted(0), ByIndex(0).Shifted(2)) // 9 - 4 = 5
        .Build();
    std::vector<uint32_t> outs = Evaluate(g);
    for (uint32_t x : outs) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
    return 0;
}