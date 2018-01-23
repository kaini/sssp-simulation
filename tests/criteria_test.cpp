#include "crit_crauser.hpp"
#include "crit_dijkstra.hpp"
#include "crit_heuristic.hpp"
#include "crit_oracle.hpp"
#include "crit_traff_bridge.hpp"
#include "graph.hpp"
#include "test_graph.hpp"
#include <boost/mpl/list.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/test/unit_test.hpp>
#include <cfloat>

using namespace sssp;

namespace {

struct crauser_in_static : crauser_in {
    crauser_in_static(const sssp::graph* g, size_t s) : crauser_in(g, s, false) {}
};

struct crauser_in_dynamic : crauser_in {
    crauser_in_dynamic(const sssp::graph* g, size_t s) : crauser_in(g, s, true) {}
};

struct crauser_out_static : crauser_out {
    crauser_out_static(const sssp::graph* g, size_t s) : crauser_out(g, s, false) {}
};

struct crauser_out_dynamic : crauser_out {
    crauser_out_dynamic(const sssp::graph* g, size_t s) : crauser_out(g, s, true) {}
};

} // namespace

using non_euclidean_tests = boost::mpl::list<traff_bridge,
                                             crauser_out_static,
                                             crauser_out_dynamic,
                                             crauser_in_static,
                                             crauser_in_dynamic,
                                             oracle,
                                             smallest_tentative_distance>;

static constexpr int test_count = 1000;

BOOST_AUTO_TEST_CASE_TEMPLATE(criteria_non_euclidean_test, Criteria, non_euclidean_tests) {
    for (int seed = 0; seed < test_count; ++seed) {
        BOOST_TEST_CONTEXT("seed = " << seed) {
            const graph g = make_test_graph(seed);

            boost::base_collection<criteria> cs;
            cs.insert(Criteria(&g, 0));
            node_map<dijkstra_result> result = dijkstra(g, 0, cs);

            cs.clear();
            cs.insert(smallest_tentative_distance(&g, 0));
            node_map<dijkstra_result> reference = dijkstra(g, 0, cs);

            for (size_t node = 0; node < g.node_count(); ++node) {
                BOOST_TEST_CONTEXT("node = " << node) {
                    BOOST_TEST(reference[node].predecessor == result[node].predecessor);
                    BOOST_TEST((reference[node].distance == result[node].distance ||
                                std::abs(reference[node].distance - result[node].distance) <= DBL_EPSILON));
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(critiera_heuristic_tests) {
    for (int seed = 0; seed < test_count; ++seed) {
        BOOST_TEST_CONTEXT("seed = " << seed) {
            const auto g = make_test_graph_euclidean(seed);

            boost::base_collection<criteria> cs;
            auto h = [&](size_t node) { return distance(std::get<1>(g)[node], std::get<1>(g)[0]); };
            cs.insert(heuristic(&std::get<0>(g), 0, h));
            // This criteria is incomplete, therefore I add another one to make it complete.
            cs.insert(smallest_tentative_distance(&std::get<0>(g), 0));
            node_map<dijkstra_result> result = dijkstra(std::get<0>(g), 0, cs);

            cs.clear();
            cs.insert(smallest_tentative_distance(&std::get<0>(g), 0));
            node_map<dijkstra_result> reference = dijkstra(std::get<0>(g), 0, cs);

            for (size_t node = 0; node < std::get<0>(g).node_count(); ++node) {
                BOOST_TEST_CONTEXT("node = " << node) {
                    BOOST_TEST(reference[node].predecessor == result[node].predecessor);
                    BOOST_TEST((reference[node].distance == result[node].distance ||
                                std::abs(reference[node].distance - result[node].distance) <= DBL_EPSILON));
                }
            }
        }
    }
}
