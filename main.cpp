#include "arguments.hpp"
#include "crauser.hpp"
#include "dijkstra.hpp"
#include "draw_graph.hpp"
#include "generate_edges.hpp"
#include "generate_positions.hpp"
#include "graph.hpp"
#include "math.hpp"
#include "optimal_phases.hpp"
#include <boost/assert.hpp>
#include <boost/date_time.hpp>
#include <cairomm/cairomm.h>
#include <iostream>
#include <mutex>
#include <random>
#include <tbb/parallel_for.h>

namespace {

std::mutex output_lock;

struct output_line {
    output_line(int run, int seed, size_t node_count, size_t reachable, int relaxation_phases)
        : run(run), seed(seed), node_count(node_count), reachable(reachable), relaxation_phases(relaxation_phases) {}

    int run;
    int seed;
    size_t node_count;
    size_t reachable;
    int relaxation_phases;
};

const std::string output_line_header("run,seed,node_count,reachable,relaxation_phases");

std::ostream& operator<<(std::ostream& out, const output_line& line) {
    out << line.run << "," << line.seed << "," << line.node_count << "," << line.reachable << ","
        << line.relaxation_phases;
    return out;
}

void run(const sssp::arguments& args, int run_number) {
    using namespace sssp;

    int seed = args.seed ^ run_number;
    std::mt19937_64 rng(seed);
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
            return;
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
            return;
    }

    switch (args.edge_gen.algorithm) {
        case edge_algorithm::planar:
            generate_planar_edges(edge_seed,
                                  args.edge_gen.planar.max_length,
                                  args.edge_gen.planar.probability,
                                  edge_cost_fn,
                                  graph,
                                  positions);
            break;
        case edge_algorithm::uniform:
            generate_uniform_edges(edge_seed, args.edge_gen.uniform.probability, edge_cost_fn, graph, positions);
            break;
        default:
            BOOST_ASSERT(false);
            return;
    }

    size_t start_node = 0;
    node_map<dijkstra_result> result;
    switch (args.algorithm) {
        case sssp_algorithm::dijkstra:
            result = sssp::dijkstra(graph, start_node);
            break;
        case sssp_algorithm::crauser_in:
            result = sssp::crauser(graph, start_node, crauser_criteria::in, false);
            break;
        case sssp_algorithm::crauser_out:
            result = sssp::crauser(graph, start_node, crauser_criteria::out, false);
            break;
        case sssp_algorithm::crauser_inout:
            result = sssp::crauser(graph, start_node, crauser_criteria::inout, false);
            break;
        case sssp_algorithm::crauser_in_dyn:
            result = sssp::crauser(graph, start_node, crauser_criteria::in, true);
            break;
        case sssp_algorithm::crauser_out_dyn:
            result = sssp::crauser(graph, start_node, crauser_criteria::out, true);
            break;
        case sssp_algorithm::crauser_inout_dyn:
            result = sssp::crauser(graph, start_node, crauser_criteria::inout, true);
            break;
        case sssp_algorithm::optimal_phases:
            result = sssp::optimal_phases(graph, start_node);
            break;
        default:
            BOOST_ASSERT(false);
            return;
    }

    int max_relaxation_phase =
        std::max_element(result.begin(),
                         result.end(),
                         [](const auto& a, const auto& b) { return a.relaxation_phase < b.relaxation_phase; })
            ->relaxation_phase;

    size_t reachable_count =
        std::count_if(result.begin(), result.end(), [](const auto& n) { return n.distance < INFINITY; });

    {
        std::lock_guard<std::mutex> lock(output_lock);

        std::cout << arguments_csv_values(args) << ","
                  << output_line(run_number, seed, graph.node_count(), reachable_count, max_relaxation_phase + 1)
                  << "\n";

        if (args.image.size() > 0) {
            node_map<node_style> node_styles = graph.make_node_map([&](size_t i) {
                node_style style;
                style.position = positions[i];
                if (result[i].relaxation_phase == -1) {
                    style.color = rgb(1.0, 0.5, 0.5);
                } else {
                    double c = 0.25 + 0.75 * static_cast<double>(result[i].relaxation_phase) / max_relaxation_phase;
                    style.color = rgb(c, c, 1.0);
                    style.text = std::to_string(result[i].relaxation_phase);
                }
                return style;
            });

            edge_map<edge_style> edge_styles = graph.make_edge_map([&](size_t source, size_t destination) {
                edge_style style;
                if (result[destination].predecessor == source) {
                    style.line_width *= 2;
                    style.color = sssp::rgb(0.25, 0.25, 1.0);
                    style.foreground = true;
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
                std::cerr << "Could not write file " << args.image << ": " << ex.what() << "\n";
            }
        }
    }
}

} // namespace

int main(int argc, char* argv[]) {
    using namespace sssp;

    boost::optional<arguments> args_opt = parse_arguments(argc, argv);
    if (!args_opt) {
        return 1;
    }
    arguments args = *args_opt;

    std::cout << "# sssp-simulation";
    for (int i = 1; i < argc; ++i) {
        std::cout << " " << argv[i];
    }
    std::cout << "\n";

    auto now = boost::posix_time::second_clock::local_time();
    std::cout << "# " << now << "\n";

    std::cout << arguments_csv_header << "," << output_line_header << "\n";

    tbb::parallel_for(0, args.runs.value(), [&](int i) { run(args, i); });

    return 0;
}
