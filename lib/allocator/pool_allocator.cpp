#include "pool_allocator.hpp"
#include <cstdlib>
#include <memory>
#include <new>

namespace lib::pmr {

PoolMemoryResource::PoolMemoryResource() {
    pool_ = static_cast<std::byte*>(std::malloc(POOL_SIZE));
    if (pool_ == nullptr) {
        throw std::bad_alloc();
    }
    allocated_ = std::vector<bool>(BLOCKS_COUNT, false);
}

void* PoolMemoryResource::do_allocate(std::size_t bytes,
                                      std::size_t alignment) {
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

void PoolMemoryResource::do_deallocate(void* p, std::size_t bytes,
                                       std::size_t alignment) {
    std::size_t starting_block =
        (static_cast<std::byte*>(p) - pool_) / BLOCK_SIZE;
    std::size_t ending_block =
        (static_cast<std::byte*>(p) + bytes - pool_ + BLOCK_SIZE - 1) /
        BLOCK_SIZE;
    std::fill(allocated_.begin() + starting_block,
              allocated_.begin() + ending_block, false);
}

PoolMemoryResource::~PoolMemoryResource() { free(pool_); }

} // namespace lib::pmr
