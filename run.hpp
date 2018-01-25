#pragma once
#include "arguments.hpp"
#include "dijkstra.hpp"
#include "math.hpp"

namespace sssp {

struct run_result {
    run_result(sssp::graph graph, node_map<vec2> positions, node_map<dijkstra_result> result)
        : graph(std::move(graph)), positions(std::move(positions)), result(std::move(result)) {}
    sssp::graph graph;
    node_map<vec2> positions;
    node_map<dijkstra_result> result;
};

// Executes a run but does not output anything.
run_result execute_run(const arguments& arguments);

} // namespace sssp
