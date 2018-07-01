#pragma once
#include "graph.hpp"
#include "math.hpp"
#include <cairomm/cairomm.h>

namespace sssp {

struct node_style {
    rgb color = rgb(1.0, 1.0, 1.0);
    vec2 position = vec2(0.5, 0.5);
    std::string text = "";
};

struct edge_style {
    rgb color = rgb(0.5, 0.5, 0.5);
    double line_width = 0.002;
    bool foreground = false;
    std::string text = "";
};

// Draws the graph in the image area from 0/0 to 1/1.
void draw_graph(const Cairo::RefPtr<Cairo::Context>& cr,
                const graph& graph,
                const node_map<node_style>& node_styles,
                const edge_map<edge_style>& edge_styles);

} // namespace sssp
