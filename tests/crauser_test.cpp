#include "crauser.hpp"
#include "generate_edges.hpp"
#include "graph.hpp"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

using namespace sssp;
namespace data = boost::unit_test::data;
namespace tt = boost::test_tools;

static const std::vector<crauser_criteria> criterias{crauser_criteria::out,
                                                     crauser_criteria::in,
                                                     crauser_criteria::inout};

BOOST_DATA_TEST_CASE(crauser_test, data::xrange(1000) * criterias, seed, criteria) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> uniform_seed(INT_MIN, INT_MAX);
    std::uniform_real_distribution<double> uniform_cost(0.0, 1.0);
    std::uniform_int_distribution<size_t> uniform_node_count(5, 300);
    std::uniform_real_distribution<double> uniform_density(1.0, 5.0);

    graph g;
    size_t node_count = uniform_node_count(rng);
    for (size_t i = 0; i < node_count; ++i) {
        g.add_node();
    }
    generate_uniform_edges(uniform_seed(rng),
                           uniform_density(rng) / node_count,
                           [&](const auto&) { return uniform_cost(rng); },
                           g,
                           std::vector<vec2>(g.node_count()));

    node_map<dijkstra_result> reference = dijkstra(g, 0);
    node_map<dijkstra_result> result = crauser(g, 0, criteria);

    for (size_t node = 0; node < g.node_count(); ++node) {
        BOOST_TEST_CONTEXT("node = " << node) {
            BOOST_TEST(reference[node].predecessor == result[node].predecessor);
            BOOST_TEST((reference[node].distance == result[node].distance ||
                        std::abs(reference[node].distance - result[node].distance) <= DBL_EPSILON));
        }
    }
}
