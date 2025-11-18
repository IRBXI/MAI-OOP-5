#pragma once

#include <cstddef>
#include <memory_resource>
#include <vector>

namespace lib::pmr {

class PoolMemoryResource : std::pmr::memory_resource {
private:
    static constexpr std::size_t POOL_SIZE = 1024 * 64; // 64Kb 2^16
    static constexpr std::size_t BLOCK_SIZE = 64;
    static constexpr std::size_t BLOCKS_COUNT =
        POOL_SIZE / BLOCK_SIZE; // 2^16 / 2^6 = 2^10 = 1024 Blocks of 64 bytes
    std::byte* pool_;
    std::vector<bool> allocated_;

    void* do_allocate(std::size_t bytes, std::size_t alignment) final;
    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) final;
    bool
    do_is_equal(const std::pmr::memory_resource& other) const noexcept final;

public:
    PoolMemoryResource();
    ~PoolMemoryResource();
};

} // namespace lib::pmr
