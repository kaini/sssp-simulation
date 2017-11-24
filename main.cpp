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

int main(int argc, char* argv[]) {
	std::vector<sssp::vec2> points = sssp::generate_poisson_disc_positions(1234, 0.10, 30); //sssp::generate_uniform_positions(1234, 60);
	sssp::graph graph;
	for (size_t i = 0; i < points.size(); ++i) {
		graph.add_node();
	}
	generate_planar_edges(graph, points, 1234, 0.20, 0.5);

	size_t start_node = 0;
	auto result = sssp::dijkstra(graph, start_node);

	auto surface = Cairo::ImageSurface::create(Cairo::FORMAT_RGB24, 800, 800);
	auto cr = Cairo::Context::create(surface);
	cr->translate(50, 50);
	cr->scale(surface->get_width() - 100, surface->get_height() - 100);

	cr->save();
	cr->set_source_rgb(1.0, 1.0, 1.0);
	cr->paint();
	cr->restore();

	std::vector<sssp::node_style> node_styles(graph.node_count());
	for (size_t i = 0; i < graph.node_count(); ++i) {
		node_styles[i].position = points[i];
		if (i == start_node) {
			node_styles[i].color = sssp::rgb{0.0, 0.0, 0.0};
		}
	}

	std::map<std::tuple<size_t, size_t>, sssp::edge_style> edge_styles;
	for (const sssp::edge_info& edge : graph.edges()) {
		sssp::edge_style style;
		if (result[edge.destination].predecessor == edge.source) {
			style.line_width *= 2;
			style.color = sssp::rgb{0.0, 0.0, 0.75};
			style.foreground = true;
		}
		edge_styles[{edge.source, edge.destination}] = style;
	}
	
	sssp::draw_graph(cr, graph, node_styles, edge_styles);

	std::string filename = "image.png";
	surface->write_to_png(filename);

    return EXIT_SUCCESS;
}
