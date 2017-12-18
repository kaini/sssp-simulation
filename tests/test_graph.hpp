#pragma once
#include "graph.hpp"
#include "math.hpp"

sssp::graph make_test_graph(int seed);
std::tuple<sssp::graph, sssp::node_map<sssp::vec2>> make_test_graph_euclidean(int seed);
