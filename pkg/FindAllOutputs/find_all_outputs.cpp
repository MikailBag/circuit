#include "find_all_outputs.h"

void Go(Params const& p, Graph const& g, std::vector<Graph>& out);

void GoStep([[maybe_unused]] Params const& p, Graph const& g, std::vector<Graph>& out) {
    Graph g2{g};
    GraphNode n;
    n.links[0] = Link::OfAdd(0, 0);
    n.links[1] = Link::OfAdd(1, 0);
    g2.nodes.push_back(n);
    out.push_back(std::move(g2));
}

void Go(Params const& p, Graph const& g, std::vector<Graph>& out) {
    if (g.nodes.size() == p.explicitNodeCountLimit) {
        out.push_back(g);
        return;
    }
    std::vector<Graph> tmp;
    GoStep(p, g, tmp);
    for (Graph& g2 : tmp) {
        Go(p, g2, out);
    }
}


void FindAllTopologies(Params const& p, std::vector<Graph>& out) {
    Graph g;

    Go(p, g, out);
}