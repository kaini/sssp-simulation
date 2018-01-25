#pragma once
#include "thread_local_allocator.hpp"
#include <boost/geometry.hpp>
#include <string>
#include <vector>

namespace sssp {

template <typename T> using tl_vector = std::vector<T, thread_local_allocator<T>>;

template <typename T> using tl_string = std::basic_string<char, std::char_traits<char>, thread_local_allocator<char>>;

template <typename Value,
          typename Parameters,
          typename IndexableGetter = boost::geometry::index::indexable<Value>,
          typename EqualTo = boost::geometry::index::equal_to<Value>>
using tl_rtree =
    boost::geometry::index::rtree<Value, Parameters, IndexableGetter, EqualTo, thread_local_allocator<Value>>;

} // namespace sssp