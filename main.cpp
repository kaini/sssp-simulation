#include "graph.hpp"
#include "dijkstra.hpp"
#include "generate_edges.hpp"
#include "generate_positions.hpp"
#include "draw_graph.hpp"
#include "math.hpp"
#include "arguments.hpp"
#include <random>
#include <iostream>
#include <cairomm/cairomm.h>

int main(int argc, char* argv[]) {
	using namespace sssp;
	
	arguments args = parse_arguments(argc, argv);

	std::mt19937 rng(args.seed);
	std::uniform_int_distribution<int> uniform_seed(INT_MIN, INT_MAX);

	node_map<vec2> positions;
	if (args.position_gen.algorithm.value() == position_poisson) {
		positions = generate_poisson_disc_positions(
			uniform_seed(rng),
			args.position_gen.poisson.min_distance,
			args.position_gen.poisson.max_reject);
	} else if (args.position_gen.algorithm.value() == position_uniform) {
		positions = generate_uniform_positions(
			uniform_seed(rng),
			args.position_gen.uniform.count);
	} else {
		assert(false);
		return 1;
	}

	graph graph;
	for (size_t i = 0; i < positions.size(); ++i) {
		graph.add_node();
	}

	auto cost_fn = [](auto&) { return 1.0; };  // TODO

	if (args.edge_gen.algorithm.value() == edge_planar) {
		generate_planar_edges(
			uniform_seed(rng),
			args.edge_gen.planar.max_length,
			args.edge_gen.planar.probability,
			cost_fn,
			graph,
			positions);
	} else if (args.edge_gen.algorithm.value() == edge_uniform) {
		generate_uniform_edges(
			uniform_seed(rng),
			args.edge_gen.uniform.probability,
			cost_fn,
			graph,
			positions);
	} else {
		assert(false);
		return 1;
	}

	size_t start_node = 0;
	node_map<dijkstra_result> result = sssp::dijkstra(graph, start_node);

	node_map<node_style> node_styles = graph.make_node_map([&](size_t i) {
		node_style style;
		style.position = positions[i];
		if (i == start_node) {
			style.color = rgb(0.0, 0.0, 0.0);
		} else if (result[i].predecessor == -1) {
			style.color = rgb(1.0, 0.5, 0.5);
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

    return 0;
}
