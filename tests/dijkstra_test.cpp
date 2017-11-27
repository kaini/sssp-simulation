#include "dijkstra.hpp"
#include "graph.hpp"
#include <boost/test/unit_test.hpp>
#include <cfloat>

using namespace sssp;
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(dijkstra_test_1) {
    graph g;
    for (size_t i = 0; i < 4; ++i) {
        g.add_node();
    }
    g.add_edge(0, 1, 1.0);
    g.add_edge(0, 2, 0.3);
    g.add_edge(2, 3, 0.3);
    g.add_edge(3, 1, 0.3);

    node_map<dijkstra_result> result = dijkstra(g, 0);

    BOOST_TEST_REQUIRE(result.size() == 4);
    BOOST_TEST(result[0].predecessor == -1);
    BOOST_TEST(result[1].predecessor == 3);
    BOOST_TEST(result[2].predecessor == 0);
    BOOST_TEST(result[3].predecessor == 2);
    BOOST_TEST(result[0].distance == 0.0, tt::tolerance(DBL_EPSILON));
    BOOST_TEST(result[1].distance == 0.9, tt::tolerance(DBL_EPSILON));
    BOOST_TEST(result[2].distance == 0.3, tt::tolerance(DBL_EPSILON));
    BOOST_TEST(result[3].distance == 0.6, tt::tolerance(DBL_EPSILON));
}

BOOST_AUTO_TEST_CASE(dijkstra_test_2) {
    graph g;
    for (size_t i = 0; i < 10; ++i) {
        g.add_node();
    }
    for (size_t i = 0; i < 10; ++i) {
        for (size_t j = 0; j < 10; ++j) {
            if (i != j) {
                g.add_edge(i, j, 1.0);
            }
        }
    }

    node_map<dijkstra_result> result = dijkstra(g, 0);

    BOOST_TEST_REQUIRE(result.size() == 10);
    BOOST_TEST(result[0].predecessor == -1);
    BOOST_TEST(result[0].distance == 0.0, tt::tolerance(DBL_EPSILON));
    for (size_t i = 1; i < 10; ++i) {
        BOOST_TEST(result[i].predecessor == 0);
        BOOST_TEST(result[i].distance == 1.0, tt::tolerance(DBL_EPSILON));
    }
}

BOOST_AUTO_TEST_CASE(dijkstra_test_3) {
    graph g;
    g.add_node();
    g.add_node();
    g.add_node();
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 0, 1.0);
    g.add_edge(2, 1, 1.0);
    g.add_edge(2, 0, 1.0);

    node_map<dijkstra_result> result = dijkstra(g, 0);

    BOOST_TEST_REQUIRE(result.size() == 3);
    BOOST_TEST(result[0].predecessor == -1);
    BOOST_TEST(result[1].predecessor == 0);
    BOOST_TEST(result[2].predecessor == -1);
    BOOST_TEST(result[0].distance == 0.0, tt::tolerance(DBL_EPSILON));
    BOOST_TEST(result[1].distance == 1.0, tt::tolerance(DBL_EPSILON));
    BOOST_TEST(result[2].distance == INFINITY);
}
