#pragma once

#include "pool_allocator.hpp"

#include <cstdlib>
#include <memory>
#include <new>

namespace lib::pmr {

template <std::size_t POOL_SIZE, std::size_t BLOCK_SIZE>
PoolMemoryResource<POOL_SIZE, BLOCK_SIZE>::PoolMemoryResource() {
    pool_ = static_cast<std::byte*>(std::malloc(POOL_SIZE));
    if (pool_ == nullptr) {
        throw std::bad_alloc();
    }
    allocated_ = std::vector<bool>(BLOCKS_COUNT, false);
}

template <std::size_t POOL_SIZE, std::size_t BLOCK_SIZE>
void* PoolMemoryResource<POOL_SIZE, BLOCK_SIZE>::do_allocate(
    std::size_t bytes, std::size_t alignment) {
    std::size_t need_blocks = (bytes + BLOCK_SIZE - 1) / BLOCK_SIZE;
    std::size_t start_of_blocks = 0;
    std::size_t cur_blocks = 0;
    for (std::size_t i = 0; i < BLOCKS_COUNT; ++i) {
        if (allocated_[i]) {
            cur_blocks = 0;
            start_of_blocks = i + 1;
            continue;
        }
        cur_blocks++;
        if (cur_blocks >= need_blocks) {
            void* potential_ptr =
                static_cast<void*>(pool_ + (start_of_blocks * BLOCK_SIZE));
            std::size_t space = cur_blocks * BLOCK_SIZE;
            void* aligned_ptr =
                std::align(alignment, bytes, potential_ptr, space);
            if (aligned_ptr) {
                std::size_t actual_start_of_blocks =
                    (static_cast<std::byte*>(aligned_ptr) - pool_) / BLOCK_SIZE;
                std::fill(allocated_.begin() + actual_start_of_blocks,
                          allocated_.begin() + start_of_blocks + cur_blocks,
                          true);
                return aligned_ptr;
            }
        }
    }
    throw std::bad_alloc();
}

template <std::size_t POOL_SIZE, std::size_t BLOCK_SIZE>
void PoolMemoryResource<POOL_SIZE, BLOCK_SIZE>::do_deallocate(
    void* p, std::size_t bytes, std::size_t alignment) {
    std::size_t starting_block =
        (static_cast<std::byte*>(p) - pool_) / BLOCK_SIZE;
    std::size_t ending_block =
        (static_cast<std::byte*>(p) + bytes - pool_ + BLOCK_SIZE - 1) /
        BLOCK_SIZE;
    std::fill(allocated_.begin() + starting_block,
              allocated_.begin() + ending_block, false);
}

template <std::size_t POOL_SIZE, std::size_t BLOCK_SIZE>
PoolMemoryResource<POOL_SIZE, BLOCK_SIZE>::~PoolMemoryResource() {
    free(pool_);
}

template <std::size_t POOL_SIZE, std::size_t BLOCK_SIZE>
bool PoolMemoryResource<POOL_SIZE, BLOCK_SIZE>::do_is_equal(
    const std::pmr::memory_resource& other) const noexcept {
    return this == &other;
}

} // namespace lib::pmr
