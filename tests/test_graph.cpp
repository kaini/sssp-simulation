#include "test_graph.hpp"
#include "generate_edges.hpp"
#include "graph.hpp"
#include "math.hpp"
#include <cstdint>
#include <random>

sssp::graph make_test_graph(int seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> uniform_seed(INT_MIN, INT_MAX);
    std::uniform_real_distribution<double> uniform_cost(0.0, 1.0);
    std::uniform_int_distribution<size_t> uniform_node_count(5, 300);
    std::uniform_real_distribution<double> uniform_density(1.0, 5.0);

    sssp::graph g;
    size_t node_count = uniform_node_count(rng);
    for (size_t i = 0; i < node_count; ++i) {
        g.add_node();
    }
    sssp::generate_uniform_edges(uniform_seed(rng),
                                 uniform_density(rng) / node_count,
                                 [&](const auto&) { return uniform_cost(rng); },
                                 g,
                                 std::vector<sssp::vec2>(g.node_count()));
    return g;
}
