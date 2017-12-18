#pragma once
#include "dijkstra.hpp"
#include "graph.hpp"
#include <functional>

namespace sssp {

using relaxation_heuristic = std::function<double(size_t node)>;

// Uses a heuristic to find nodes to relax.
// The heuristic has to *underestimate* the real cost, i.e., est_fn(node) <= distance(start, node).
node_map<dijkstra_result>
heuristic_relaxing_dijkstra(const graph& graph, size_t start_node, relaxation_heuristic est_fn);

} // namespace sssp