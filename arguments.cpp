#include "arguments.hpp"
#include <boost/program_options.hpp>
#include <iostream>

sssp::arguments sssp::parse_arguments(int argc, char* argv[]) {
	namespace po = boost::program_options;

	arguments args;

	po::options_description pos_opts("Node positions");
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

	po::options_description all_opts("Single Source Shortest Path simulation tool");
	all_opts.add_options()
		("help,h", "Show this help message.")
		("seed,s", po::value(&args.seed)->default_value(args.seed), "Set the seed.");
	;
	all_opts.add(pos_opts);

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