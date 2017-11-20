#include "graph.hpp"
#include "dijkstra.hpp"
#include "generate_edges.hpp"
#include "math.hpp"
#include <cstdlib>
#include <iostream>
#include <random>
#include <cairomm/cairomm.h>
#include <cmath>

int main(int argc, char* argv[]) {
	using namespace sssp;

	graph graph;
	std::mt19937_64 rng(1234);
	std::uniform_real_distribution<double> zero_one_dist(0.0, 1.0);

	std::vector<vec2> points;
	for (size_t i = 0; i < 60; ++i) {
		points.emplace_back(vec2{zero_one_dist(rng), zero_one_dist(rng)});
		graph.add_node();
	}

	generate_planar_edges(graph, points, 1234, 0.25, 0.5);

	auto surface = Cairo::ImageSurface::create(Cairo::FORMAT_RGB24, 800, 800);
	auto cr = Cairo::Context::create(surface);
	cr->scale(surface->get_width(), surface->get_height());

	cr->save();
	cr->set_source_rgb(1.0, 1.0, 1.0);
	cr->paint();
	cr->restore();

	cr->save();
	cr->set_line_width(0.002);
	cr->set_source_rgb(0.0, 0.0, 0.0);
	for (size_t i = 0; i < graph.node_count(); ++i) {
		for (const edge_info& edge : graph.outgoing_edges(i)) {
			cr->move_to(points[i].x, points[i].y);
			cr->line_to(points[edge.destination].x, points[edge.destination].y);
			cr->stroke();
		}
	}
	cr->restore();

	cr->save();
	cr->set_line_width(0.002);
	for (size_t i = 0; i < points.size(); ++i) {
		cr->arc(points[i].x, points[i].y, 0.015, 0.0, 2.0 * M_PI);
		cr->set_source_rgba(1.0, 1.0, 1.0, 0.5);
		cr->fill_preserve();
		cr->set_source_rgba(0.0, 0.0, 0.0, 0.5);
		cr->stroke();
	}
	cr->restore();

	std::string filename = "image.png";
	surface->write_to_png(filename);

    return EXIT_SUCCESS;
}
