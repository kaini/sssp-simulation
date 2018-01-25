#include "buddy_allocator.hpp"
#include <boost/test/unit_test.hpp>

using namespace sssp;
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(buddy_allocator_vector_test) {
    buddy_allocator_memory allocator_memory;
    std::vector<double, buddy_allocator<double>> vector(&allocator_memory);

    for (int i = 0; i < 1000000; ++i) {
        vector.push_back(i);
    }

    for (int i = 0; i < 1000000; ++i) {
        BOOST_TEST_REQUIRE(vector[i] == i);
    }

    vector.clear();
    vector.shrink_to_fit();

    for (int i = 0; i < 1000000; ++i) {
        vector.push_back(-i);
    }

    for (int i = 0; i < 1000000; ++i) {
        BOOST_TEST_REQUIRE(vector[i] == -i);
    }
}
