#pragma once
#include "dijkstra.hpp"
#include <cstdlib>

namespace sssp {

// Relaxes each node that is reached via its shortest path.
node_map<dijkstra_result> optimal_phases(const graph& graph, size_t start_node);

} // namespace sssp
