#include "arguments.hpp"
#include <boost/program_options.hpp>
#include <iostream>

sssp::arguments sssp::parse_arguments(int argc, char* argv[]) {
	namespace po = boost::program_options;

	arguments args;

	po::options_description pos_opts("Node positions (note that all nodes are always in the area from 0/0 to 1/1)");
	pos_opts.add_options()
		("position-gen,P", po::value(&args.position_gen.algorithm)->default_value(args.position_gen.algorithm),
			"Set the generator used to generate node positions. Possible values:\n  - poisson: \tpoisson-disc sampling\n  - uniform: \tuniform sampling")
		("Ppoisson-min-distance", po::value(&args.position_gen.poisson.min_distance)->default_value(args.position_gen.poisson.min_distance),
			"Set the minimal distance between points. (> 0)")
		("Ppoisson-max-reject", po::value(&args.position_gen.poisson.max_reject)->default_value(args.position_gen.poisson.max_reject),
			"Set the maximal number of times before giving up to sample an annulus. (> 0)")
		("Puniform-count", po::value(&args.position_gen.uniform.count)->default_value(args.position_gen.uniform.count),
			"Set the number of nodes to generate. (> 0)")
		;

	po::options_description edge_opts("Edges");
	edge_opts.add_options()
		("edge-gen,E", po::value(&args.edge_gen.algorithm)->default_value(args.edge_gen.algorithm),
			"Set the generator used to generate edges. Possible values:\n  - planar: \tthe graph will be planar\n  - uniform: \trandom edges")
		("Eplanar-max-length", po::value(&args.edge_gen.planar.max_length)->default_value(args.edge_gen.planar.max_length),
			"Set the maximum distance of nodes considered when looking for possible edges to add. (> 0)")
		("Eplanar-probability", po::value(&args.edge_gen.planar.probability)->default_value(args.edge_gen.planar.probability),
			"Set the probability that a valid edge is added. (>= 0; <= 1)")
		("Euniform-probability", po::value(&args.edge_gen.uniform.probability)->default_value(args.edge_gen.uniform.probability),
			"Set the probability for each edge to be added. (>= 0; <= 1)")
		;

	po::options_description cost_opts("Edge costs");
	cost_opts.add_options()
		("cost-gen,C", po::value(&args.cost_gen.algorithm)->default_value(args.cost_gen.algorithm),
			"Set the generator used to generate edge costs. Possible values:\n - uniform: \tthe edge costs are uniformly random between 0 and 1\n - one: \tall edges have cost 1\n - euclidean: \tall edges have their euclidean length as cost")
		;

	po::options_description all_opts("Single Source Shortest Path simulation tool. Global options");
	all_opts.add_options()
		("help,h", "Show this help message.")
		("seed,s", po::value(&args.seed)->default_value(args.seed), "Set the seed.");
	;
	all_opts.add(pos_opts);
	all_opts.add(edge_opts);
	all_opts.add(cost_opts);

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, all_opts), vm);
		po::notify(vm);
	} catch (const boost::program_options::error& ex) {
		std::cerr << ex.what() << "\n";
		exit(1);  // TODO remove
	}

	if (vm.count("help")) {
		std::cout << all_opts << "\n";
		exit(1);  // TODO remove
	}

	return args;
}