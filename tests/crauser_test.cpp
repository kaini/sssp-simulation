#include "crauser.hpp"
#include "graph.hpp"
#include "test_graph.hpp"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <cfloat>

using namespace sssp;
namespace data = boost::unit_test::data;

static const std::vector<crauser_criteria> criterias{crauser_criteria::out,
                                                     crauser_criteria::in,
                                                     crauser_criteria::inout};
static const std::vector<bool> dynamics{false, true};

BOOST_DATA_TEST_CASE(crauser_test, data::xrange(1000) * criterias * dynamics, seed, criteria, dynamic) {
    graph g = make_test_graph(seed);

    node_map<dijkstra_result> result = crauser(g, 0, criteria, dynamic);

    node_map<dijkstra_result> reference = dijkstra(g, 0);
    for (size_t node = 0; node < g.node_count(); ++node) {
        BOOST_TEST_CONTEXT("node = " << node) {
            BOOST_TEST(reference[node].predecessor == result[node].predecessor);
            BOOST_TEST((reference[node].distance == result[node].distance ||
                        std::abs(reference[node].distance - result[node].distance) <= DBL_EPSILON));
        }
    }
}
