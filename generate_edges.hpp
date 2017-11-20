#pragma once
#include "graph.hpp"
#include "math.hpp"
#include <vector>

namespace sssp {

void generate_planar_edges(graph& graph, const std::vector<vec2>& positions, int seed, double max_edge_length, double edge_probability);

}
