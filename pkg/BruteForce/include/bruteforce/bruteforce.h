#pragma once

#include "params.h"
#include "topology.h"

#include <vector>

namespace bf {
namespace detail {
class BruteforceEntrypoints {
public:
    static std::vector<Topology> FindTopologies(FindTopologyParams const& p);
    static std::vector<Topology> FilterTopologies(FilterParams const& p, std::span<Topology const> topologies);
    static std::vector<int64_t> EvalTopologies(EvalParams const& p, std::vector<Topology> const& topologies);
private:
    BruteforceEntrypoints() = delete;
};
}
inline std::vector<Topology> FindTopologies(FindTopologyParams const& p) {
    return detail::BruteforceEntrypoints::FindTopologies(p);
}

inline std::vector<Topology> FilterTopologies(FilterParams const& p, std::span<Topology const> topologies) {
    return detail::BruteforceEntrypoints::FilterTopologies(p, topologies);
}

inline std::vector<int64_t> EvalTopologies(EvalParams const& p, std::vector<Topology> const& topologies) {
    return detail::BruteforceEntrypoints::EvalTopologies(p, topologies);
}

}