#include "generate_edges.hpp"
#include <random>

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
    std::bernoulli_distribution allow_edge(edge_probability);

    for (size_t source = 0; source < graph.node_count(); ++source) {
        for (size_t destination = 0; destination < graph.node_count(); ++destination) {
            if (destination != source && allow_edge(rng)) {
                graph.add_edge(source, destination, edge_cost(line(positions[source], positions[destination])));
            }
        }
    }
}
