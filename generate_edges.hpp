#pragma once
#include "graph.hpp"
#include "math.hpp"
#include <vector>

namespace sssp {

using edge_cost_fn = std::function<double(const line&)>;

// Generates edges that do not intersect each other. The max_edge_length parameter is the radius
// that is considered for creating edges and edge_probability is the chance that a possible edge
// is chosen.
void generate_planar_edges(int seed,
                           double max_edge_length,
                           double edge_probability,
                           const edge_cost_fn& edge_cost,
                           graph& graph,
                           const node_map<vec2>& positions);

// Generates uniformly random edges. The positions are irrelevant and only used for the cost callback.
void generate_uniform_edges(int seed,
                            double edge_probability,
                            const edge_cost_fn& edge_cost,
                            graph& graph,
                            const node_map<vec2>& positions);

} // namespace sssp
