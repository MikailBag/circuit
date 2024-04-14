#include "bruteforce/topology.h"

#include <stdexcept>

namespace bf {
void ValidateTopology(const Topology &t, size_t inputCount) {
    for (size_t i = 0; i < t.nodes.size(); i++) {
        if (t.nodes[i].links[0] >= i + inputCount || t.nodes[i].links[1] >= i + inputCount) {
            throw std::invalid_argument("invalid links");
        }
    }
}
}