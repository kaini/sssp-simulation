#include "graph.hpp"
#include "dijkstra.hpp"
#include "generate_edges.hpp"
#include "generate_positions.hpp"
#include "draw_graph.hpp"
#include "math.hpp"
#include <cstdlib>
#include <iostream>
#include <random>
#include <cairomm/cairomm.h>
#include <cmath>

using namespace sssp;

int main(int argc, char* argv[]) {
	node_map<vec2> positions = sssp::generate_poisson_disc_positions(1234, 0.10, 30); //sssp::generate_uniform_positions(1234, 60);

	sssp::graph graph;
	for (size_t i = 0; i < positions.size(); ++i) {
		graph.add_node();
	}

	generate_planar_edges(graph, positions, 1234, 0.20, 0.5);

	size_t start_node = 0;
	node_map<dijkstra_result> result = sssp::dijkstra(graph, start_node);

	node_map<node_style> node_styles = graph.make_node_map([&](size_t i) {
		node_style style;
		style.position = positions[i];
		if (i == start_node) {
			style.color = rgb(0.0, 0.0, 0.0);
		}
		return style;
	});

	edge_map<edge_style> edge_styles = graph.make_edge_map([&](size_t source, size_t destination) {
		edge_style style;
		if (result[destination].predecessor == source) {
			style.line_width *= 2;
			style.color = sssp::rgb{0.0, 0.0, 0.75};
			style.foreground = true;
		}
		return style;
	});
	
	auto surface = Cairo::ImageSurface::create(Cairo::FORMAT_RGB24, 800, 800);
	auto cr = Cairo::Context::create(surface);
	cr->translate(50, 50);
	cr->scale(surface->get_width() - 100, surface->get_height() - 100);
	cr->save();
	cr->set_source_rgb(1.0, 1.0, 1.0);
	cr->paint();
	cr->restore();
	draw_graph(cr, graph, node_styles, edge_styles);
	surface->write_to_png("image.png");

    return EXIT_SUCCESS;
}
