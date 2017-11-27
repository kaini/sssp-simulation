#include "math.hpp"
#include <boost/test/unit_test.hpp>
#include <cfloat>

using namespace sssp;
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(distance_test) {
    vec2 a(1, 1);
    vec2 b(2, 3);

    double d1 = distance(a, b);
    double d2 = distance(b, a);

    BOOST_TEST(d1 == 2.2360679774997896964091736687313, tt::tolerance(DBL_EPSILON));
    BOOST_TEST(d2 == 2.2360679774997896964091736687313, tt::tolerance(DBL_EPSILON));
}

BOOST_AUTO_TEST_CASE(intersects_positive_test) {
    line a({-1, 0}, {1, 0});
    line b({0, 1}, {0, -1});

    BOOST_TEST(intersects(a, b));
    BOOST_TEST(intersects(b, a));
}

BOOST_AUTO_TEST_CASE(intersects_too_short_test) {
    line a({-1, 0}, {-0.0001, 0});
    line b({0, 1}, {0, -1});

    BOOST_TEST(!intersects(a, b));
    BOOST_TEST(!intersects(b, a));
}

BOOST_AUTO_TEST_CASE(intersects_parallel_test) {
    line a({0, 0}, {1000, 1000});
    line b({1, 0}, {1001, 1000});

    BOOST_TEST(!intersects(a, b));
    BOOST_TEST(!intersects(b, a));
}
