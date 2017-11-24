#pragma once
#include "graph.hpp"
#include "math.hpp"
#include <map>
#include <tuple>
#include <cairomm/cairomm.h>

namespace sssp {

struct node_style {
	rgb color = {1.0, 1.0, 1.0};
	vec2 position = {0.5, 0.5};
};

struct edge_style {
	rgb color = {0.5, 0.5, 0.5};
	double line_width = 0.002;
	bool foreground = false;
};

// Draws the graph in the image area from 0/0 to 1/1.
void draw_graph(
	const Cairo::RefPtr<Cairo::Context>& cr,
	const graph& graph,
	const std::vector<node_style>& node_styles,
	const std::map<std::tuple<size_t, size_t>, edge_style>& edge_styles
);

}
