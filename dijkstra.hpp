#pragma once
#include "arguments.hpp"
#include "criteria.hpp"
#include "graph.hpp"
#include <boost/poly_collection/base_collection.hpp>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace sssp {

struct dijkstra_result {
    size_t predecessor = size_t(-1);
    double distance = INFINITY;
    int relaxation_phase = -1;
    size_t fringe_size = -1;

    bool unexplored() const { return relaxation_phase == -1 && predecessor == size_t(-1); }
    bool fringe() const { return relaxation_phase == -1 && predecessor != size_t(-1); }
    bool settled() const { return relaxation_phase != -1; }
};

node_map<dijkstra_result> dijkstra(const graph& graph, size_t start_node, boost::base_collection<criteria>& criteria);

} // namespace sssp
