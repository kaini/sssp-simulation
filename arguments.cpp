#include "arguments.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <set>
#include <sstream>

// clang-format off
extern const std::string sssp::arguments_csv_header(
    "position_alg,position_poisson_min_distance,position_poisson_max_reject,position_uniform_count,"
    "edge_alg,edge_planar_probability,edge_uniform_probability,edge_layered_probability,edge_layered_count,edge_kronecker_initiator,edge_kronecker_k,"
    "cost_alg,"
    "graph_file,"
    "alg,seed");
// clang-format on

namespace sssp { // thanks ADL!
template <typename T> static std::string na_if(bool na, const T& value) {
    if (na) {
        return "NA";
    } else {
        std::ostringstream out;
        out << value;
        return out.str();
    }
}
} // namespace sssp

std::ostream& sssp::operator<<(std::ostream& out, const std::vector<sssp_algorithm>& algorithms) {
    bool first = true;
    for (const auto& algorithm : algorithms) {
        if (!first) {
            out << "+";
        } else {
            first = false;
        }
        out << algorithm;
    }
    return out;
}

std::ostream& sssp::operator<<(std::ostream& out, const std::vector<double>& vector) {
    bool first = true;
    for (const auto& value : vector) {
        if (!first) {
            out << ";";
        } else {
            first = false;
        }
        out << value;
    }
    return out;
}

std::string sssp::arguments_csv_values(const sssp::arguments& args) {
    using namespace sssp;

    bool graph_file = !args.graph_file.empty();
    bool kronecker = args.edge_gen.algorithm == edge_algorithm::kronecker;

    std::ostringstream out;
    out << na_if(graph_file || kronecker, args.position_gen.algorithm) << ",";
    out << na_if(args.position_gen.algorithm != position_algorithm::poisson || graph_file || kronecker,
                 args.position_gen.poisson.min_distance)
        << ",";
    out << na_if(args.position_gen.algorithm != position_algorithm::poisson || graph_file || kronecker,
                 args.position_gen.poisson.max_reject)
        << ",";
    out << na_if(args.position_gen.algorithm != position_algorithm::uniform || graph_file || kronecker,
                 args.position_gen.uniform.count)
        << ",";
    out << na_if(graph_file, args.edge_gen.algorithm) << ",";
    out << na_if(args.edge_gen.algorithm != edge_algorithm::planar || graph_file, args.edge_gen.planar.probability)
        << ",";
    out << na_if(args.edge_gen.algorithm != edge_algorithm::uniform || graph_file, args.edge_gen.uniform.probability)
        << ",";
    out << na_if(args.edge_gen.algorithm != edge_algorithm::layered || graph_file, args.edge_gen.layered.probability)
        << ",";
    out << na_if(args.edge_gen.algorithm != edge_algorithm::layered || graph_file, args.edge_gen.layered.count) << ",";
    out << na_if(args.edge_gen.algorithm != edge_algorithm::kronecker || graph_file, args.edge_gen.kronecker.initiator)
        << ",";
    out << na_if(args.edge_gen.algorithm != edge_algorithm::kronecker || graph_file, args.edge_gen.kronecker.k) << ",";
    out << na_if(graph_file, args.cost_gen.algorithm) << ",";
    out << na_if(!graph_file, boost::filesystem::path(args.graph_file).filename().string()) << ",";
    out << args.algorithms << ",";
    out << na_if(graph_file, args.seed);

    return out.str();
}

boost::optional<sssp::arguments> sssp::parse_arguments(int argc, const char* const* argv, std::ostream* error_output) {
    namespace po = boost::program_options;

    arguments args;

    std::ostringstream oss;
    oss << args.edge_gen.kronecker.initiator;
    std::string kronecker_initiator(oss.str());

    // clang-format off

    po::options_description pos_opts("Node positions (note that all nodes are always in the area from 0/0 to 1/1)");
    pos_opts.add_options()
        ("position-gen,P", po::value(&args.position_gen.algorithm)->default_value(args.position_gen.algorithm),
            "Set the generator used to generate node positions. Possible values:\n"
            "  - poisson: \tpoisson-disc sampling\n"
            "  - uniform: \tuniform sampling")
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
            "Set the generator used to generate edges. Possible values:\n"
            "  - planar: \tthe graph will be planar\n"
            "  - uniform: \trandom edges\n"
            "  - layered: \tlayered graph (e.g. bipartite)\n"
            "  - kronecker: \tKronecker graphs as defined in Leskovec et al. 2010. If you choose Kronecker graphs, all options regarting the node positions are ignored.")
        ("Eplanar-probability", po::value(&args.edge_gen.planar.probability)->default_value(args.edge_gen.planar.probability),
            "Set the probability that a valid edge is added. (>= 0; <= 1)")
        ("Euniform-probability", po::value(&args.edge_gen.uniform.probability)->default_value(args.edge_gen.uniform.probability),
            "Set the probability for each edge to be added. (>= 0; <= 1)")
        ("Elayered-probability", po::value(&args.edge_gen.layered.probability)->default_value(args.edge_gen.layered.probability),
            "Set the probability for each edge to be added. (>= 0; <= 1)")
        ("Elayered-count", po::value(&args.edge_gen.layered.count)->default_value(args.edge_gen.layered.count),
            "Set the number of layers. A value of 2 generates a bipartite graph. (> 0)")
        ("Ekronecker-initiator", po::value(&kronecker_initiator)->default_value(kronecker_initiator),
            "Set the initiator matrix. (square size and each entry >= 0; <= 1)")
        ("Ekronecker-k", po::value(&args.edge_gen.kronecker.k)->default_value(args.edge_gen.kronecker.k),
            "Set the Kronecker power. The generated graph has at most initiator-size^k nodes. (> 0)")
        ;

    po::options_description cost_opts("Edge costs");
    cost_opts.add_options()
        ("cost-gen,C", po::value(&args.cost_gen.algorithm)->default_value(args.cost_gen.algorithm),
            "Set the generator used to generate edge costs. Possible values:\n"
            "  - uniform: \tthe edge costs are uniformly random between 0 and 1\n"
            "  - one: \tall edges have cost 1\n"
            "  - euclidean: \tall edges have their euclidean length as cost")
        ;

    po::options_description all_opts("Single Source Shortest Path simulation tool. Global options");
    all_opts.add_options()
        ("help,h",
            "Show this help message.")
        ("graph-file,g", po::value(&args.graph_file)->default_value(args.graph_file),
            "Load a graph from a textfile with the line-format `source destination cost`, with cost being optional. Position, edge and cost generation choices are ignored if this option is supplied.")
        ("seed,s", po::value(&args.seed)->default_value(args.seed),
            "Set the seed.")
        ("algorithm,a", po::value<std::vector<sssp_algorithm>>(&args.algorithms)->composing()->default_value(args.algorithms),
            "Set the SSSP algorithm. This argument can be passed multiple times to combine criteria. Possible values:\n"
            "  - dijkstra: \tDijkstra's algorithm\n"
            "  - crauser_in: \tCrauser et al. using the IN criteria.\n"
            "  - crauser_in_dyn: \tCrauser et al. using the IN criteria only looking at nodes not settled.\n"
            "  - crauser_out: \tCrauser et al. using the OUT criteria.\n"
            "  - crauser_out_dyn: \tCrauser et al. using the OUT criteria only looking at nodes not settled.\n"
            "  - oracle: \tUses an oracle to relax all nodes that can be safely relaxed in any given phase.\n"
            "  - heuristic: \tUses a heuristic to decide which nodes can be relaxed. The graph has to be euclidean.\n"
            "  - traff: \tLike Crauser et al. IN dynamic critiera but with an additional static lookahead.")
#ifndef DISABLE_CAIRO
        ("image,i", po::value(&args.image)->default_value(""),
            "If set to a filename, output an image (PDF-file) displaying the graph and visualizing the algorithm.")
#endif
        ;
    all_opts.add(pos_opts);
    all_opts.add(edge_opts);
    all_opts.add(cost_opts);

    // clang-format on

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, all_opts), vm);
        po::notify(vm);
    } catch (const boost::program_options::error& ex) {
        if (error_output) {
            *error_output << ex.what() << "\n";
        }
        return {};
    }

    if (vm.count("help")) {
        if (error_output) {
            *error_output << all_opts << "\n";
        }
        return {};
    }

    if (std::find(args.algorithms.begin(), args.algorithms.end(), sssp_algorithm::heuristic) != args.algorithms.end() &&
        args.cost_gen.algorithm != cost_algorithm::euclidean) {
        if (error_output) {
            *error_output << "`-a heuristic` cannot be used without `-C euclidean`.\n";
        }
        return {};
    }

    std::set<sssp_algorithm> algorithms_set(args.algorithms.begin(), args.algorithms.end());
    args.algorithms = std::vector<sssp_algorithm>(algorithms_set.begin(), algorithms_set.end());

    args.edge_gen.kronecker.initiator.clear();
    std::vector<std::string> split_kronecker_initiator;
    boost::split(split_kronecker_initiator,
                 kronecker_initiator,
                 [](char c) { return c == ';'; },
                 boost::algorithm::token_compress_on);
    for (const auto& cell : split_kronecker_initiator) {
        try {
            double value = boost::lexical_cast<double>(cell);
            if (value < 0 || value > 1) {
                if (error_output) {
                    *error_output << "entries in `--Ekronecker-initiator` must be between 0 and 1.\n";
                }
                return {};
            }
            args.edge_gen.kronecker.initiator.push_back(value);
        } catch (boost::bad_lexical_cast&) {
            if (error_output) {
                *error_output << "invalid matrix passed to `--Ekronecker-initiator`: cannot parse number.\n";
            }
            return {};
        }
    }
    size_t i = 1;
    while (i * i != args.edge_gen.kronecker.initiator.size()) {
        if (i * i > args.edge_gen.kronecker.initiator.size()) {
            if (error_output) {
                *error_output << "matrix passed to `--Ekronecker-initiator` must be square.\n";
            }
            return {};
        }
        i += 1;
    }

    return args;
}

boost::optional<sssp::arguments>
sssp::parse_arguments(const char* argv0, const std::vector<std::string>& raw_args, std::ostream* error_output) {
    std::vector<const char*> argv;
    argv.reserve(raw_args.size() + 1);
    argv.push_back(argv0);
    for (const std::string& arg : raw_args) {
        argv.push_back(arg.c_str());
    }
    return parse_arguments(static_cast<int>(argv.size()), argv.data(), error_output);
}
