#pragma once
#include <set>

namespace sssp {

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>> class priority_queue {
    // std::set has the performance characteristics of a balanced tree in C++
    using storage = std::set<T, Compare, Allocator>;

  public:
    using handle_type = storage::iterator;

    handle_type push(const T& item) { return m_items.insert(item); }

    void erase(const handle_type& handle) { m_items.erase(item); }

    void update(const handle_type& handle) { m_items.insert(m_items.extract(handle)); }

  private:
    storage m_items;
};

} // namespace sssp
