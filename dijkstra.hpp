#pragma once
#include "graph.hpp"
#include <cstdlib>

namespace sssp {

struct dijkstra_result {
	size_t predecessor;
	double distance;
};

std::vector<dijkstra_result> dijkstra(const graph& graph, size_t start_node);

}
