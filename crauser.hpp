#pragma once
#include "dijkstra.hpp"
#include "graph.hpp"
#include "stringy_enum.hpp"

namespace sssp {

STRINGY_ENUM(crauser_criteria, in, out, inout)

// Implements Crauser's algorithm with either the IN, INOUT or OUT criteria. Additionally
// instead of using the minimal edges, with dynamic=true, one can use the minimal none-
// relaxed edge.
node_map<dijkstra_result> crauser(const graph& graph, size_t start_node, crauser_criteria criteria, bool dynamic);

} // namespace sssp