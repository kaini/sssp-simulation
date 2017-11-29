#include "generate_edges.hpp"
#include "partial_shuffle.hpp"
#include <random>
#include <unordered_set>

void sssp::generate_planar_edges(int seed,
                                 double max_edge_length,
                                 double edge_probability,
                                 edge_cost_fn edge_cost,
                                 graph& graph,
                                 const node_map<vec2>& positions) {
    std::mt19937 rng(seed);
    std::bernoulli_distribution allow_edge(edge_probability);

    std::vector<line> lines;

    for (size_t source = 0; source < graph.node_count(); ++source) {
        for (size_t destination = 0; destination < graph.node_count(); ++destination) {
            if (destination != source && distance(positions[source], positions[destination]) <= max_edge_length &&
                allow_edge(rng)) {
                line this_line(positions[source], positions[destination]);

                bool does_intersect = false;
                for (const line& line : lines) {
                    if (this_line.start != line.start && this_line.end != line.start && this_line.start != line.end &&
                        this_line.end != line.end && intersects(line, this_line)) {
                        does_intersect = true;
                        break;
                    }
                }

                if (!does_intersect) {
                    graph.add_edge(source, destination, edge_cost(this_line));
                    lines.emplace_back(this_line);
                }
            }
        }
    }
}

void sssp::generate_uniform_edges(int seed,
                                  double edge_probability,
                                  edge_cost_fn edge_cost,
                                  graph& graph,
                                  const node_map<vec2>& positions) {
    std::mt19937 rng(seed);

#if 0
    // This is the naive n*(n-1) steps algorithm. Each possible edge is considered
    // using a simple Bernulli(p) distribution.
    std::bernoulli_distribution allow_edge(edge_probability);
    for (size_t source = 0; source < graph.node_count(); ++source) {
        for (size_t destination = 0; destination < graph.node_count(); ++destination) {
            if (destination != source && allow_edge(rng)) {
                graph.add_edge(source, destination, edge_cost(line(positions[source], positions[destination])));
            }
        }
    }
#else
    // An more efficient variant is to draw for each node k ~ Binom(n-1, p), and then
    // generate edges to k random other nodes.
    std::binomial_distribution<size_t> edge_count_dist(graph.node_count() - 1, edge_probability);
    node_map<size_t> destinations = graph.make_node_map([](size_t i) { return i; });
    for (size_t source = 0; source < graph.node_count(); ++source) {
        size_t edge_count = edge_count_dist(rng);
        partial_shuffle(destinations.begin(), destinations.begin() + edge_count + 1, destinations.end(), rng);
        for (size_t i = 0; i < edge_count; ++i) {
            size_t destination;
            if (destinations[i] == source) {
                // Note that I shuffled k+1 elements above, for exactly this case. If one of the
                // randomly chosen edges is the source itself I use the (k+1)th element.
                destination = destinations[edge_count];
            } else {
                destination = destinations[i];
            }
            graph.add_edge(source, destination, edge_cost(line(positions[source], positions[destination])));
        }
    }
#endif
}
