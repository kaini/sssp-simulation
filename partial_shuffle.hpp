#pragma once
#include <algorithm>
#include <iterator>
#include <random>

namespace sssp {

template <typename Iter, typename Rng> void partial_shuffle(Iter begin, Iter shuffle_end, Iter end, Rng& rng) {
    size_t distance = std::distance(begin, end);
    size_t at_index = 0;
    Iter at = begin;
    while (at != shuffle_end) {
        size_t i = std::uniform_int_distribution<size_t>(at_index, distance - 1)(rng);
        std::swap(*at, *(begin + i));
        ++at;
        ++at_index;
    }
}

} // namespace sssp
