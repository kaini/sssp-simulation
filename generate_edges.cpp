#include "generate_edges.hpp"
#include "partial_shuffle.hpp"
#include <algorithm>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <random>
#include <unordered_set>

namespace {

struct indexed_vec2 {
    indexed_vec2(size_t index, const sssp::vec2& pos) : index(index), x(pos.x), y(pos.y) {}
    size_t index;
    double x;
    double y;
};

} // namespace

BOOST_GEOMETRY_REGISTER_POINT_2D(indexed_vec2, double, boost::geometry::cs::cartesian, x, y)

void sssp::generate_uniform_edges(int seed,
                                  double edge_probability,
                                  const edge_cost_fn& edge_cost,
                                  graph& graph,
                                  const node_map<vec2>& positions) {
    std::mt19937 rng(seed);

#if 0
    // This is the naive n*(n-1) steps algorithm. Each possible edge is considered
    // using a simple Bernulli(p) distribution.
    std::bernoulli_distribution allow_edge(edge_probability);
    for (size_t source = 0; source < graph.node_count(); ++source) {
        for (size_t destination = 0; destination < graph.node_count(); ++destination) {
            if (destination != source && allow_edge(rng)) {
                graph.add_edge(source, destination, edge_cost(line(positions[source], positions[destination])));
            }
        }
    }
#else
    // An more efficient variant is to draw for each node k ~ Binom(n-1, p), and then
    // generate edges to k random other nodes.
    std::binomial_distribution<size_t> edge_count_dist(graph.node_count() - 1, edge_probability);
    node_map<size_t> destinations = graph.make_node_map([](size_t i) { return i; });
    for (size_t source = 0; source < graph.node_count(); ++source) {
        size_t edge_count = edge_count_dist(rng);
        partial_shuffle(destinations.begin(), destinations.begin() + edge_count + 1, destinations.end(), rng);
        for (size_t i = 0; i < edge_count; ++i) {
            size_t destination;
            if (destinations[i] == source) {
                // Note that I shuffled k+1 elements above, for exactly this case. If one of the
                // randomly chosen edges is the source itself I use the (k+1)th element.
                destination = destinations[edge_count];
            } else {
                destination = destinations[i];
            }
            graph.add_edge(source, destination, edge_cost(line(positions[source], positions[destination])));
        }
    }
#endif
}

void sssp::generate_planar_edges(int seed,
                                 double edge_probability,
                                 const edge_cost_fn& edge_cost,
                                 graph& graph,
                                 const node_map<vec2>& positions) {
    using namespace boost::geometry;

    std::mt19937 rng(seed);
    std::binomial_distribution<size_t> edge_count_dist(graph.node_count() - 1, edge_probability);

    index::rtree<indexed_vec2, index::quadratic<16>> points;
    for (size_t i = 0; i < positions.size(); ++i) {
        points.insert(indexed_vec2(i, positions[i]));
    }

    index::rtree<line, index::quadratic<16>> lines;
    std::vector<indexed_vec2> result;

    for (size_t source = 0; source < graph.node_count(); ++source) {
        int edge_count = static_cast<int>(edge_count_dist(rng));
        if (edge_count > 0) {
            result.clear();
            // I find the 2 * edge_count closest nodes and pick edge_count random from them.
            points.query(index::nearest(indexed_vec2(-1, positions[source]), edge_count * 2),
                         std::back_inserter(result));
            std::shuffle(result.begin(), result.end(), rng);
            int done = 0;
            for (const auto& dest : result) {
                if (done >= edge_count) {
                    break;
                }
                if (dest.index == source) {
                    continue;
                }
                line candidate(positions[source], positions[dest.index]);
                auto line_endings_unequal = [&](const line& l) {
                    return l.start != candidate.start && l.end != candidate.start && l.start != candidate.end &&
                           l.end != candidate.end;
                };
                if (lines.qbegin(index::intersects(candidate) && index::satisfies(line_endings_unequal)) ==
                    lines.qend()) {
                    graph.add_edge(source, dest.index, edge_cost(candidate));
                    lines.insert(candidate);
                    done += 1;
                }
            }
        }
    }
}
