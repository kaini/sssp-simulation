#include "graph.hpp"
#include "heuristic_relaxing_dijkstra.hpp"
#include "test_graph.hpp"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <cfloat>

using namespace sssp;
namespace data = boost::unit_test::data;

BOOST_DATA_TEST_CASE(heuristic_relaxing_dijkstra_test, data::xrange(1000), seed) {
    graph g;
    node_map<vec2> positions;
    std::tie(g, positions) = make_test_graph_euclidean(seed);

    node_map<dijkstra_result> result =
        heuristic_relaxing_dijkstra(g, 0, [&](size_t i) { return distance(positions[0], positions[i]); });

    node_map<dijkstra_result> reference = dijkstra(g, 0);
    for (size_t node = 0; node < g.node_count(); ++node) {
        BOOST_TEST_CONTEXT("node = " << node) {
            BOOST_TEST(reference[node].predecessor == result[node].predecessor);
            BOOST_TEST((reference[node].distance == result[node].distance ||
                        std::abs(reference[node].distance - result[node].distance) <= DBL_EPSILON));
        }
    }
}
