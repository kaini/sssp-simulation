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

    boost::geometry::index::rtree<indexed_vec2, index::quadratic<16>> points;
    for (size_t i = 0; i < positions.size(); ++i) {
        points.insert(indexed_vec2(i, positions[i]));
    }

    boost::geometry::index::rtree<line, index::quadratic<16>> lines;
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

static int y_bucket(int layers, double y) {
    BOOST_ASSERT(layers >= 1);
    BOOST_ASSERT(0.0 <= y && y <= 1.0);
    // The std::min is just for the rare case that some y is exactly 1.
    return std::min(static_cast<int>(std::floor(y * layers)), layers - 1);
}

void sssp::generate_layered_edges(int seed,
                                  double edge_probability,
                                  int layers,
                                  const edge_cost_fn& edge_cost,
                                  graph& graph,
                                  const node_map<vec2>& positions) {
    std::mt19937 rng(seed);

    std::vector<std::vector<size_t>> buckets(layers);
    for (size_t n = 0; n < graph.node_count(); ++n) {
        buckets[y_bucket(layers, positions[n].y)].emplace_back(n);
    }

    for (size_t source = 0; source < graph.node_count(); ++source) {
        std::vector<size_t> destinations;
        int source_bucket = y_bucket(layers, positions[source].y);
        if (source_bucket > 0) {
            destinations.insert(
                destinations.end(), buckets[source_bucket - 1].begin(), buckets[source_bucket - 1].end());
        }
        if (source_bucket < layers - 1) {
            destinations.insert(
                destinations.end(), buckets[source_bucket + 1].begin(), buckets[source_bucket + 1].end());
        }
        size_t edge_count = std::binomial_distribution<size_t>(destinations.size(), edge_probability)(rng);
        partial_shuffle(destinations.begin(), destinations.begin() + edge_count, destinations.end(), rng);
        for (size_t i = 0; i < edge_count; ++i) {
            graph.add_edge(source, destinations[i], edge_cost(line(positions[source], positions[destinations[i]])));
        }
    }
}

void sssp::generate_kronecker_graph(int seed,
                                    size_t start_size,
                                    int k,
                                    const edge_cost_fn& edge_cost,
                                    graph& graph,
                                    node_map<vec2>& positions) {
    std::mt19937 rng(seed);

    size_t final_size = 1;
    for (int i = 0; i < k; ++i) {
        final_size *= start_size;
    }
    positions.resize(final_size, vec2(0.0, 0.0));
    for (size_t n = 0; n < final_size; ++n) {
        graph.add_node();
    }

    std::vector<double> matrix(start_size * start_size);
    auto p1 = [&](size_t x, size_t y) { return matrix[x + y * start_size]; };
    std::uniform_real_distribution<double> uniform01(0.0, 1.0);
    for (double& entry : matrix) {
        entry = uniform01(rng);
    }

    std::vector<double> matrix_prefix_sum(start_size * start_size);
    matrix_prefix_sum[0] = matrix[0];
    for (int i = 1; i < matrix.size(); ++i) {
        matrix_prefix_sum[i] = matrix_prefix_sum[i - 1] + matrix[i];
    }

    // The matrix can be interpret as parameters of a poisson binomial distribution
    // The Kronecker product multiplies the expected values:
    // Consider: M = [a b; c d] and M' = [a' b'; c' d'].
    // The Kronecker product leads to [aM' bM'; cM' dM'], therefore the new sum is
    // sum(aM') + sum(bM') + sum(cM') + sum(dM') = (a + b + c + d) * sum(M') = sum(M) * sum(M').
    double edges_expected_value = std::pow(matrix_prefix_sum.back(), k);
    // Now the poisson bionmial distribution can be apprximated well by a poisson distribution if
    // the probabilites are very low (this is given here due to the nature of potentiation of lots of
    // probabilities).
    size_t edges = std::poisson_distribution<size_t>(edges_expected_value)(rng);
    // Realistically this edge case can only happen with very low node counts.
    edges = std::min(edges, final_size);

    std::uniform_real_distribution<double> cell_dist(0.0, matrix_prefix_sum.back());
    std::unordered_set<size_t> used_cells;
    for (size_t e = 0; e < edges; ++e) {
        // Sample the cell for the edge
        size_t cell = 0;
        size_t granularity = 1;
        for (int i = 0; i < k; ++i) {
            double value = cell_dist(rng);
            size_t index = std::distance(matrix_prefix_sum.begin(),
                                         std::lower_bound(matrix_prefix_sum.begin(), matrix_prefix_sum.end(), value));
            if (index == matrix_prefix_sum.size()) {
                // This should not happen, but could happen due to double inaccuracies
                index = matrix_prefix_sum.size() - 1;
            }
            cell += index * granularity;
            granularity *= start_size * start_size;
        }
        size_t u = cell / final_size;
        size_t v = cell % final_size;
        if (u == v || used_cells.find(cell) != used_cells.end()) {
            // Try again
            e -= 1;
            continue;
        }
        used_cells.insert(cell);
        graph.add_edge(u, v, edge_cost(line(vec2(0.0, 0.0), vec2(0.0, 0.0))));
    }
}
