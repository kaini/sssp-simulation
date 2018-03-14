#pragma once
#include "graph.hpp"
#include "math.hpp"
#include <vector>

namespace sssp {

using edge_cost_fn = std::function<double(const line&)>;

// Generates uniformly random edges. The positions are irrelevant and only used for the cost callback.
void generate_uniform_edges(int seed,
                            double edge_probability,
                            const edge_cost_fn& edge_cost,
                            graph& graph,
                            const node_map<vec2>& positions);

// Generates edges that do not intersect each other. The max_edge_length parameter is the radius
// that is considered for creating edges and edge_probability is the chance that a possible edge
// is chosen.
void generate_planar_edges(int seed,
                           double edge_probability,
                           const edge_cost_fn& edge_cost,
                           graph& graph,
                           const node_map<vec2>& positions);

// Creates graphs that can only be connected like that: 1 <-> 2 <-> 3 ...
// with 1, 2, 3 ... being the layers.
void generate_layered_edges(int seed,
                            double edge_probability,
                            int layers,
                            const edge_cost_fn& edge_cost,
                            graph& graph,
                            const node_map<vec2>& positions);

// Creates a Kronecker graph. Different to the other functions, this function also creates the nodes
// in the graph and fills the positions node_map with all zeros. Note that this implements
// the stochastic version of the algorithm.
void generate_kronecker_graph(int seed,
                              size_t start_size,
                              int k,
                              const edge_cost_fn& edge_cost,
                              graph& graph,
                              node_map<vec2>& positions);

} // namespace sssp
