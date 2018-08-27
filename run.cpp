#include "crit_crauser.hpp"
#include "crit_dijkstra.hpp"
#include "crit_heuristic.hpp"
#include "crit_oracle.hpp"
#include "crit_traff_bridge.hpp"
#include "dijkstra.hpp"
#include "generate_edges.hpp"
#include "generate_positions.hpp"
#include "math.hpp"
#include "run.hpp"
#include <boost/algorithm/string.hpp>
#include <cctype>
#include <fstream>
#include <random>

#ifndef DISABLE_CAIRO
#include "draw_graph.hpp"
#include <cairomm/cairomm.h>
#endif

using namespace boost::algorithm;

const std::string sssp::dijkstra_result_csv_header("node_count,phase,relaxed");

void sssp::execute_run(const arguments& args, std::ostream* out, std::ostream* err) {
    size_t start_node = 0;
    node_map<vec2> positions;
    graph graph;

    if (args.graph_file.empty()) {
        std::mt19937_64 rng(args.seed);
        std::uniform_int_distribution<int> uniform_seed(INT_MIN, INT_MAX);
        int position_seed = uniform_seed(rng);
        int cost_seed = uniform_seed(rng);
        int edge_seed = uniform_seed(rng);

        if (args.edge_gen.algorithm != edge_algorithm::kronecker) {
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

            for (size_t i = 0; i < positions.size(); ++i) {
                graph.add_node();
            }
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
            case edge_algorithm::kronecker:
                generate_kronecker_graph(edge_seed,
                                         args.edge_gen.kronecker.initiator,
                                         args.edge_gen.kronecker.k,
                                         edge_cost_fn,
                                         graph,
                                         positions);
                break;
            default:
                BOOST_ASSERT(false);
                break;
        }
    } else {
        std::ifstream in(args.graph_file);
        std::string line;
        std::vector<std::string> columns;
        std::set<std::tuple<std::string, std::string, double>> edges;
        std::unordered_map<std::string, size_t> nodes;
        size_t node_index = 0;
        while (std::getline(in, line)) {
            trim(line);
            if (line.empty() || starts_with(line, "#") || starts_with(line, "//") || starts_with(line, "--")) {
                continue;
            }

            columns.clear();
            split(columns, line, [](char c) { return std::isspace(static_cast<int>(c)); }, token_compress_on);
            if (columns.size() < 2) {
                if (err) {
                    (*err) << "Parse error in " << args.graph_file << "!\n";
                }
                return;
            }

            double cost = 1.0;
            if (columns.size() >= 3) {
                try {
                    cost = boost::lexical_cast<double>(columns[2]);
                } catch (const boost::bad_lexical_cast& ex) {
                    if (err) {
                        (*err) << "Parse error in " << args.graph_file << ": " << ex.what() << "\n";
                    }
                    return;
                }
            }

            edges.emplace(columns[0], columns[1], cost);
            if (nodes.find(columns[0]) == nodes.end()) {
                nodes[columns[0]] = node_index;
                node_index += 1;
                graph.add_node();
            }
            if (nodes.find(columns[1]) == nodes.end()) {
                nodes[columns[1]] = node_index;
                node_index += 1;
                graph.add_node();
            }
        }
        if (!in.eof()) {
            if (err) {
                (*err) << "Read error: " << args.graph_file << "\n";
            }
            return;
        }

        if (graph.node_count() == 0) {
            return; // nothing to do ...
        }

        for (const auto& edge : edges) {
            graph.add_edge(nodes[std::get<0>(edge)], nodes[std::get<1>(edge)], std::get<double>(edge));
        }
        positions = graph.make_node_map([](size_t) { return vec2(0.0, 0.0); });
    }

    if (graph.node_count() == 0) {
        (*err) << "The generated graph is empty!\n";
        return;
    }

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

    node_map<dijkstra_result> result = dijkstra(graph, start_node, criteria);

    size_t reachable = 0;
    int max_phase = 0;
    std::vector<size_t> relaxed(graph.node_count(), 0);
    for (const auto& r : result) {
        if (r.settled()) {
            reachable += 1;
            max_phase = std::max(max_phase, r.relaxation_phase);
            relaxed[r.relaxation_phase] += 1;
        }
    }

    if (out) {
        for (int phase = 0; phase <= max_phase; ++phase) {
            *out << arguments_csv_values(args) << "," << graph.node_count() << "," << phase << "," << relaxed[phase]
                 << "\n";
        }
    }

#ifndef DISABLE_CAIRO
    if (args.image.size() > 0) {
        int layers = 1;
        std::vector<size_t> count_by_layer(1, graph.node_count());
        std::vector<size_t> placed_by_layer(1, 0);
        if (args.edge_gen.algorithm == edge_algorithm::layered) {
            layers = args.edge_gen.layered.count;
            count_by_layer = std::vector<size_t>(layers, 0);
            placed_by_layer = std::vector<size_t>(layers, 0);
            for (size_t node = 0; node < graph.node_count(); ++node) {
                count_by_layer[y_bucket(layers, positions[node].y)] += 1;
            }
        }

        node_map<node_style> node_styles = graph.make_node_map([&](size_t i) {
            node_style style;

            style.position = positions[i];
            if (result[i].relaxation_phase == -1) {
                style.color = rgb(1.0, 0.5, 0.5);
            } else {
                double c = 0.25 + 0.75 * static_cast<double>(result[i].relaxation_phase) / max_phase;
                style.color = rgb(c, c, 1.0);
                style.text = std::to_string(result[i].relaxation_phase);
            }

            // Override values if disabled
            if (!args.image_node_labels) {
                style.text = "";
            }
            if (!args.image_node_colors) {
                style.color = rgb(1.0, 1.0, 1.0);
            }
            if (args.edge_gen.algorithm == edge_algorithm::layered) {
                int layer = y_bucket(layers, positions[i].y);
                style.position.x = (1.0 / (count_by_layer[layer] + 1)) * (placed_by_layer[layer] + 1);
                style.position.y = (1.0 / (layers + 1)) * (layer + 1);
                placed_by_layer[layer] += 1;
            }

            return style;
        });

        edge_map<edge_style> edge_styles = graph.make_edge_map([&](size_t source, size_t destination) {
            edge_style style;

            for (const edge_info& edge : graph.outgoing_edges(source)) {
                if (edge.destination == destination) {
                    style.text = std::to_string(int(std::round(edge.cost * 1000))) + ">";
                    break;
                }
            }

            if (result[destination].predecessor == source) {
                style.line_width *= 2;
                style.color = rgb(0.25, 0.25, 1.0);
                style.foreground = true;
            }

            // Override values if disabled
            if (!args.image_edge_colors) {
                style.color = rgb(0.0, 0.0, 0.0);
                style.line_width = edge_style().line_width;
                style.foreground = edge_style().foreground;
            }
            if (!args.image_edge_labels) {
                style.text = "";
            }

            return style;
        });

        try {
            int width = 800;
            int height = 800;
            auto surface = Cairo::PdfSurface::create(args.image, width, height);
            auto cr = Cairo::Context::create(surface);
            cr->translate(25, 25);
            cr->scale(width - 50, height - 50);
            cr->save();
            cr->set_source_rgb(1.0, 1.0, 1.0);
            cr->paint();
            cr->restore();
            draw_graph(cr, graph, node_styles, edge_styles);
        } catch (const std::ios_base::failure& ex) {
            if (err) {
                *err << "Could not write file " << args.image << ": " << ex.what() << "\n";
            }
        }
    }
#endif
}
