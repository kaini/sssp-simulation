#include "arguments.hpp"
#include "dijkstra.hpp"
#include "draw_graph.hpp"
#include "generate_edges.hpp"
#include "generate_positions.hpp"
#include "graph.hpp"
#include "math.hpp"
#include <boost/assert.hpp>
#include <cairomm/cairomm.h>
#include <iostream>
#include <random>

int main(int argc, char* argv[]) {
    using namespace sssp;

    arguments args = parse_arguments(argc, argv);

    std::mt19937_64 rng(args.seed);
    std::uniform_int_distribution<int> uniform_seed(INT_MIN, INT_MAX);
    int position_seed = uniform_seed(rng);
    int cost_seed = uniform_seed(rng);
    int edge_seed = uniform_seed(rng);

    node_map<vec2> positions;
    if (args.position_gen.algorithm.value() == position_poisson) {
        positions = generate_poisson_disc_positions(
            position_seed, args.position_gen.poisson.min_distance, args.position_gen.poisson.max_reject);
    } else if (args.position_gen.algorithm.value() == position_uniform) {
        positions = generate_uniform_positions(position_seed, args.position_gen.uniform.count);
    } else {
        BOOST_ASSERT(false);
        return 1;
    }

    graph graph;
    for (size_t i = 0; i < positions.size(); ++i) {
        graph.add_node();
    }

    edge_cost_fn edge_cost_fn;
    if (args.cost_gen.algorithm.value() == cost_uniform) {
        edge_cost_fn = [rng = std::mt19937(cost_seed)](const line& line) mutable {
            return std::uniform_real_distribution<double>(0.0, 1.0)(rng);
        };
    } else if (args.cost_gen.algorithm.value() == cost_one) {
        edge_cost_fn = [](const line& line) { return 1.0; };
    } else if (args.cost_gen.algorithm.value() == cost_euclidean) {
        edge_cost_fn = [](const line& line) { return distance(line.start, line.end); };
    } else {
        BOOST_ASSERT(false);
        return 1;
    }

    if (args.edge_gen.algorithm.value() == edge_planar) {
        generate_planar_edges(edge_seed,
                              args.edge_gen.planar.max_length,
                              args.edge_gen.planar.probability,
                              edge_cost_fn,
                              graph,
                              positions);
    } else if (args.edge_gen.algorithm.value() == edge_uniform) {
        generate_uniform_edges(edge_seed, args.edge_gen.uniform.probability, edge_cost_fn, graph, positions);
    } else {
        BOOST_ASSERT(false);
        return 1;
    }

    size_t start_node = 0;
    node_map<dijkstra_result> result = sssp::dijkstra(graph, start_node);

    int max_relaxation_phase =
        std::max_element(result.begin(),
                         result.end(),
                         [](const auto& a, const auto& b) { return a.relaxation_phase < b.relaxation_phase; })
            ->relaxation_phase;

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

    int width = 800;
    int height = 800;
    auto surface = Cairo::PdfSurface::create("image.pdf", width, height);
    auto cr = Cairo::Context::create(surface);
    cr->translate(25, 25);
    cr->scale(width - 50, height - 50);
    cr->save();
    cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->paint();
    cr->restore();
    draw_graph(cr, graph, node_styles, edge_styles);

    return 0;
}
