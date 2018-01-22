#pragma once
#include <boost/assert.hpp>
#include <memory>
#include <vector>

namespace sssp {

struct local_linear_allocator_control_block {
    local_linear_allocator_control_block(size_t chunk_size) : chunk_size(chunk_size) {}
    const size_t chunk_size;
    std::vector<std::unique_ptr<char[]>> blocks;
    size_t block_at = 0;
    size_t byte_at = 0;
};

// None-threadsafe linear memory allocator. Never frees memory once the last copy of this
// allocator is destructed.
template <typename T> class local_linear_allocator {
  public:
    using value_type = T;

    template <typename OtherT> friend class local_linear_allocator;

    // ctor
    local_linear_allocator(size_t chunk_size)
        : m_c(std::make_shared<local_linear_allocator_control_block>(chunk_size)) {}

    // rebind conversion
    template <typename OtherT> local_linear_allocator(const local_linear_allocator<OtherT>& other) : m_c(other.m_c) {}

    // move & copy ctor
    local_linear_allocator(const local_linear_allocator<T>& other) : m_c(other.m_c) {}
    local_linear_allocator(const local_linear_allocator<T>&& other) : m_c(other.m_c) {}
    local_linear_allocator<T>& operator=(const local_linear_allocator<T>& other) {
        if (*other != this) {
            m_c = other.m_c;
        }
        return *this;
    }
    local_linear_allocator<T>& operator=(const local_linear_allocator<T>&& other) {
        if (*other != this) {
            m_c = other.m_c;
        }
        return *this;
    }

    template <typename OtherT> struct rebind { using other = local_linear_allocator<OtherT>; };

    T* allocate(size_t n) const {
        local_linear_allocator_control_block& c = *m_c;
        while (true) {
            if (c.block_at == c.blocks.size()) {
                c.blocks.emplace_back(new char[c.chunk_size]);
            }

            size_t address = c.byte_at + reinterpret_cast<size_t>(c.blocks.back().get());
            size_t real_size = sizeof(T) * n;
            if (address % alignof(T) != 0) {
                size_t offset = alignof(T) - (address % alignof(T));
                address += offset;
                real_size += offset;
            }

            if (c.byte_at + real_size <= c.chunk_size) {
                c.byte_at += real_size;
                return reinterpret_cast<T*>(address);
            } else if (c.byte_at == 0) {
                // The requested allocation is too big for the ChunkSize
                throw std::bad_alloc();
            } else {
                // We need a new chunk
                c.byte_at = 0;
                c.block_at += 1;
            }
        }
    }

    void deallocate(T* ptr, size_t n) const {}

    template <typename OtherT> bool operator==(const local_linear_allocator<OtherT>& other) const {
        return m_c == other.m_c;
    }
    template <typename OtherT> bool operator!=(const local_linear_allocator<OtherT>& other) const {
        return m_c != other.m_c;
    }

  private:
    std::shared_ptr<local_linear_allocator_control_block> m_c;
};

} // namespace sssp
