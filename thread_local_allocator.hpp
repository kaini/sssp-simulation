#pragma once
#include "buddy_allocator.hpp"

namespace sssp {

extern thread_local buddy_allocator_memory thread_local_allocator_memory;

// A thread local buddy allocator. This allocator is stateless.
// Do *not* allocate anything with it that might escape a thread,
// your program will break in very fancy ways!!!
template <typename T> class thread_local_allocator {
  public:
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using void_pointer = void*;
    using const_void_pointer = const void*;
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    template <typename OtherT> struct rebind { using other = thread_local_allocator<OtherT>; };

    thread_local_allocator() = default;
    template <typename OtherT> thread_local_allocator(const thread_local_allocator<OtherT>& other) {}

    bool operator==(const thread_local_allocator& other) const { return true; }
    bool operator!=(const thread_local_allocator& other) const { return false; }

    T* allocate(size_t n) { return buddy_allocator<T>(&thread_local_allocator_memory).allocate(n); }
    void deallocate(T* ptr, size_t n) { buddy_allocator<T>(&thread_local_allocator_memory).deallocate(ptr, n); }
};

} // namespace sssp