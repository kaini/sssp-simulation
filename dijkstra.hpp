#pragma once
#include "arguments.hpp"
#include "criteria.hpp"
#include "graph.hpp"
#include <boost/poly_collection/base_collection.hpp>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace sssp {

extern const std::string dijkstra_result_csv_header;

struct dijkstra_result {
    size_t predecessor = size_t(-1);
    double distance = INFINITY;
    int relaxation_phase = -1;

    bool unexplored() const { return relaxation_phase == -1 && predecessor == size_t(-1); }
    bool fringe() const { return relaxation_phase == -1 && predecessor != size_t(-1); }
    bool settled() const { return relaxation_phase != -1; }
};

struct dijkstra_result_csv_values {
    dijkstra_result_csv_values(const node_map<dijkstra_result>& result);

    size_t node_count;
    size_t reachable;
    int relaxation_phases;
};

std::ostream& operator<<(std::ostream& out, const dijkstra_result_csv_values& line);

node_map<dijkstra_result> dijkstra(const graph& graph, size_t start_node, boost::base_collection<criteria>& criteria);

} // namespace sssp
