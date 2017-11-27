#pragma once
#include "dijkstra.hpp"
#include "graph.hpp"
#include "stringy_enum.hpp"

namespace sssp {

STRINGY_ENUM(crauser_criteria, in, out, inout)

node_map<dijkstra_result> crauser(const graph& graph, size_t start_node, crauser_criteria criteria);

} // namespace sssp