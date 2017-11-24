#include "generate_edges.hpp"
#include <random>

void sssp::generate_planar_edges(graph& graph, const node_map<vec2>& positions, int seed, double max_edge_length, double edge_probability) {
	std::mt19937 rng(seed);
	std::bernoulli_distribution allow_edge(edge_probability);

	std::vector<line> lines;

	for (size_t source = 0; source < graph.node_count(); ++source) {
		for (size_t destination = 0; destination < graph.node_count(); ++destination) {
			if (destination != source &&
					distance(positions[source], positions[destination]) <= max_edge_length &&
					allow_edge(rng)) {
				line this_line(positions[source], positions[destination]);

				bool does_intersect = false;
				for (const line& line : lines) {
					if (this_line.start != line.start && this_line.end != line.start &&
						this_line.start != line.end && this_line.end != line.end &&
						intersects(line, this_line)) {
						does_intersect = true;
						break;
					}
				}
				
				if (!does_intersect) {
					graph.add_edge(source, destination, 1.0);  // TODO edge cost
					lines.emplace_back(this_line);
				}
			}
		}
	}
}
