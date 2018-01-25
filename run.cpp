#include "run.hpp"
#include "crit_crauser.hpp"
#include "crit_dijkstra.hpp"
#include "crit_heuristic.hpp"
#include "crit_oracle.hpp"
#include "crit_traff_bridge.hpp"
#include "dijkstra.hpp"
#include "generate_edges.hpp"
#include "generate_positions.hpp"
#include "math.hpp"
#include <random>

sssp::run_result sssp::execute_run(const arguments& args) {
    std::mt19937_64 rng(args.seed);
    std::uniform_int_distribution<int> uniform_seed(INT_MIN, INT_MAX);
    int position_seed = uniform_seed(rng);
    int cost_seed = uniform_seed(rng);
    int edge_seed = uniform_seed(rng);

    node_map<vec2> positions;
    switch (args.position_gen.algorithm) {
        case position_algorithm::poisson:
            positions = generate_poisson_disc_positions(
                position_seed, args.position_gen.poisson.min_distance, args.position_gen.poisson.max_reject);
            break;
        case position_algorithm::uniform:
            positions = generate_uniform_positions(position_seed, args.position_gen.uniform.count);
            break;
        default:
            BOOST_ASSERT(false);
            break;
    }

    graph graph;
    for (size_t i = 0; i < positions.size(); ++i) {
        graph.add_node();
    }

    edge_cost_fn edge_cost_fn;
    switch (args.cost_gen.algorithm) {
        case cost_algorithm::uniform:
            edge_cost_fn = [rng = std::mt19937(cost_seed)](const line& line) mutable {
                return std::uniform_real_distribution<double>(0.0, 1.0)(rng);
            };
            break;
        case cost_algorithm::one:
            edge_cost_fn = [](const line& line) { return 1.0; };
            break;
        case cost_algorithm::euclidean:
            edge_cost_fn = [](const line& line) { return distance(line.start, line.end); };
            break;
        default:
            BOOST_ASSERT(false);
            break;
    }

    switch (args.edge_gen.algorithm) {
        case edge_algorithm::planar:
            generate_planar_edges(edge_seed, args.edge_gen.planar.probability, edge_cost_fn, graph, positions);
            break;
        case edge_algorithm::uniform:
            generate_uniform_edges(edge_seed, args.edge_gen.uniform.probability, edge_cost_fn, graph, positions);
            break;
        case edge_algorithm::layered:
            generate_layered_edges(edge_seed,
                                   args.edge_gen.layered.probability,
                                   args.edge_gen.layered.count,
                                   edge_cost_fn,
                                   graph,
                                   positions);
            break;
        default:
            BOOST_ASSERT(false);
            break;
    }

    size_t start_node = 0;
    boost::base_collection<criteria> criteria;
    for (sssp_algorithm crit : args.algorithms) {
        switch (crit) {
            case sssp_algorithm::crauser_in:
                criteria.insert(crauser_in(&graph, start_node, false));
                break;
            case sssp_algorithm::crauser_in_dyn:
                criteria.insert(crauser_in(&graph, start_node, true));
                break;
            case sssp_algorithm::crauser_out:
                criteria.insert(crauser_out(&graph, start_node, false));
                break;
            case sssp_algorithm::crauser_out_dyn:
                criteria.insert(crauser_out(&graph, start_node, true));
                break;
            case sssp_algorithm::dijkstra:
                criteria.insert(smallest_tentative_distance(&graph, start_node));
                break;
            case sssp_algorithm::heuristic:
                criteria.insert(heuristic(
                    &graph, start_node, [&](size_t node) { return distance(positions[start_node], positions[node]); }));
                break;
            case sssp_algorithm::oracle:
                criteria.insert(oracle(&graph, start_node));
                break;
            case sssp_algorithm::traff:
                criteria.insert(traff_bridge(&graph, start_node));
                break;
            default:
                BOOST_ASSERT(false);
                break;
        }
    }

    auto result = dijkstra(graph, start_node, criteria);

    return run_result(std::move(graph), std::move(positions), std::move(result));
}
