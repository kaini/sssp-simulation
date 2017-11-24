#include "graph.hpp"
#include "dijkstra.hpp"
#include "generate_edges.hpp"
#include "generate_positions.hpp"
#include "draw_graph.hpp"
#include "math.hpp"
#include <random>
#include <iostream>
#include <cairomm/cairomm.h>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

namespace {

struct arguments {
	struct position_gen {
		enum class alg {
			poisson,
			uniform,
		} algorithm;
		struct poisson {
			double min_distance;
			int max_reject;
		} poisson;
		struct uniform {
			int count = 60;
		} uniform;
	} position_gen;

	int seed;
};

arguments parse_arguments(int argc, char* argv[]) {
	namespace po = boost::program_options;

	po::options_description pos_opts("Node positions");
	pos_opts.add_options()
		("position-gen,P", po::value<std::string>()->default_value("poisson")->value_name("alg"),
			"Set the generator used to generate node positions. Possible values:\n  poisson: \tpoisson-disc sampling\n  uniform: \tuniform sampling")
		("Ppoisson-min-distance", po::value<std::string>()->default_value("0.1")->value_name("r"),
			"Set the minimal distance between points.")
		("Ppoisson-max-reject", po::value<int>()->default_value(30)->value_name("k"),
			"Set the maximal number of times before giving up to sample an annulus.")
		("Puniform-count", po::value<int>()->default_value(60)->value_name("n"),
			"Set the number of nodes to generate.")
		;

	po::options_description all_opts("Single Source Shortest Path simulation tool");
	all_opts.add_options()
		("help,h", "Show this help message.")
		("seed,s", po::value<int>()->default_value(42)->value_name("seed"), "Set the seed.");
	;
	all_opts.add(pos_opts);

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, all_opts), vm);
		po::notify(vm);
	} catch (const boost::program_options::error& ex) {
		std::cerr << ex.what() << "\n";
		exit(1);
	}

	if (vm.count("help")) {
		std::cout << all_opts << "\n";
		exit(1);
	}

	arguments args;

	if (vm["position-gen"].as<std::string>() == "poisson") {
		args.position_gen.algorithm = arguments::position_gen::alg::poisson;
	} else if (vm["position-gen"].as<std::string>() == "uniform") {
		args.position_gen.algorithm = arguments::position_gen::alg::uniform;
	} else {
		std::cerr << "Unknown '--position-gen'.\n";
		exit(1);
	}

	args.position_gen.poisson.min_distance = boost::lexical_cast<double>(vm["Ppoisson-min-distance"].as<std::string>());
	if (args.position_gen.poisson.min_distance <= 0.0) {
		std::cerr << "'--Ppoisson-min-distance' must be positive.\n";
		exit(1);
	}

	args.position_gen.poisson.max_reject = vm["Ppoisson-max-reject"].as<int>();
	if (args.position_gen.poisson.max_reject < 1) {
		std::cerr << "'--Ppoisson-max-reject' must be positive.\n";
		exit(1);
	}

	args.position_gen.uniform.count = vm["Puniform-count"].as<int>();
	if (args.position_gen.uniform.count < 1) {
		std::cerr << "'--Puniform-count' must be positive.\n";
		exit(1);
	}

	args.seed = vm["seed"].as<int>();

	return args;
}

}

int main(int argc, char* argv[]) {
	using namespace sssp;
	
	arguments args = parse_arguments(argc, argv);

	std::mt19937 rng(args.seed);
	std::uniform_int_distribution<int> uniform_seed(INT_MIN, INT_MAX);

	node_map<vec2> positions;
	switch (args.position_gen.algorithm) {
	case arguments::position_gen::alg::poisson:
		positions = generate_poisson_disc_positions(uniform_seed(rng), args.position_gen.poisson.min_distance, args.position_gen.poisson.max_reject);
		break;
	case arguments::position_gen::alg::uniform:
		positions = generate_uniform_positions(uniform_seed(rng), args.position_gen.uniform.count);
		break;
	default:
		assert(false);
		break;
	}

	graph graph;
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

    return 0;
}
