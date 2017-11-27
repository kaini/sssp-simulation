#include "graph.hpp"
#include <boost/test/unit_test.hpp>

using namespace sssp;
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(graph_test) {
    graph g;
    g.add_node();
    g.add_node();
    g.add_node();

    BOOST_TEST_REQUIRE(g.node_count() == 3);

    g.add_edge(0, 1, 1234.5);
    g.add_edge(0, 2, 1.2345);
    g.add_edge(1, 2, 1.0);

    BOOST_TEST_REQUIRE(g.incoming_edges(0).size() == 0);
    BOOST_TEST_REQUIRE(g.incoming_edges(1).size() == 1);
    BOOST_TEST_REQUIRE(g.incoming_edges(2).size() == 2);
    BOOST_TEST_REQUIRE(g.outgoing_edges(0).size() == 2);
    BOOST_TEST_REQUIRE(g.outgoing_edges(1).size() == 1);
    BOOST_TEST_REQUIRE(g.outgoing_edges(2).size() == 0);

    BOOST_TEST(g.incoming_edges(1)[0].cost == 1234.5);
    BOOST_TEST(g.incoming_edges(1)[0].source == 0);
    BOOST_TEST(g.incoming_edges(1)[0].destination == 1);

    BOOST_TEST(g.outgoing_edges(1)[0].cost == 1.0);
    BOOST_TEST(g.outgoing_edges(1)[0].source == 1);
    BOOST_TEST(g.outgoing_edges(1)[0].destination == 2);
}

BOOST_AUTO_TEST_CASE(graph_node_map_test) {
    graph g;
    g.add_node();
    g.add_node();
    g.add_node();

    node_map<size_t> result = g.make_node_map([](size_t i) { return i; });

    BOOST_TEST_REQUIRE(result.size() == 3);
    BOOST_TEST(result[0] == 0);
    BOOST_TEST(result[1] == 1);
    BOOST_TEST(result[2] == 2);
}

BOOST_AUTO_TEST_CASE(graph_edge_map_test) {
    graph g;
    g.add_node();
    g.add_node();
    g.add_node();
    g.add_edge(0, 1, 1.0);
    g.add_edge(2, 0, 2.0);
    g.add_edge(1, 2, 3.0);

    edge_map<size_t> result = g.make_edge_map([](size_t s, size_t d) { return 10 * s + d; });

    BOOST_TEST_REQUIRE(result.size() == 3);
    BOOST_TEST((result[{0, 1}]) == 1);
    BOOST_TEST((result[{2, 0}]) == 20);
    BOOST_TEST((result[{1, 2}]) == 12);
}
