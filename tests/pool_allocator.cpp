#include <cstddef>
#include <gtest/gtest.h>
#include <vector>

#include "pool_allocator.hpp"

namespace lib::pmr {
namespace {

// Helper struct to hold pool configuration
template <std::size_t PoolSize, std::size_t BlockSize>
struct PoolConfig {
    static constexpr std::size_t kPoolSize = PoolSize;
    static constexpr std::size_t kBlockSize = BlockSize;
    static constexpr std::size_t kNumBlocks = PoolSize / BlockSize;
};

// Test fixture for parameterized tests
template <typename Config>
class PoolMemoryResourceTest : public ::testing::Test {
protected:
    using ConfigType = Config;
    PoolMemoryResource<Config::kPoolSize, Config::kBlockSize> resource;

    // Helper to calculate blocks needed for a size
    std::size_t blocks_needed(std::size_t size) {
        return (size + Config::kBlockSize - 1) / Config::kBlockSize;
    }
};

// Define test configurations
using TestConfigurations = ::testing::Types<PoolConfig<1024, 64>,  // 16 blocks
                                            PoolConfig<1024, 128>, // 8 blocks
                                            PoolConfig<2048, 256>, // 8 blocks
                                            PoolConfig<4096, 512>  // 8 blocks
                                            >;

TYPED_TEST_SUITE(PoolMemoryResourceTest, TestConfigurations);

TYPED_TEST(PoolMemoryResourceTest, ConstructorDestructor) { SUCCEED(); }

TYPED_TEST(PoolMemoryResourceTest, AllocateSingleBlock) {
    void* block = this->resource.allocate(TypeParam::kBlockSize, 1);
    this->resource.deallocate(block, TypeParam::kBlockSize, 1);
}

TYPED_TEST(PoolMemoryResourceTest, AllocateMultipleContiguousBlocks) {
    constexpr std::size_t multi_block_size = TypeParam::kBlockSize * 2;
    const std::size_t blocks_needed = this->blocks_needed(multi_block_size);

    void* block = this->resource.allocate(multi_block_size, 1);

    // Should be able to allocate remaining blocks
    const std::size_t remaining_blocks = TypeParam::kNumBlocks - blocks_needed;
    std::vector<void*> other_blocks;
    for (std::size_t i = 0; i < remaining_blocks; ++i) {
        void* b = this->resource.allocate(TypeParam::kBlockSize, 1);
        other_blocks.push_back(b);
    }

    // Over-allocate should throw
    EXPECT_THROW(this->resource.allocate(TypeParam::kBlockSize, 1),
                 std::bad_alloc);

    this->resource.deallocate(block, multi_block_size, 1);
    for (void* b : other_blocks) {
        this->resource.deallocate(b, TypeParam::kBlockSize, 1);
    }
}

TYPED_TEST(PoolMemoryResourceTest, ReuseSingleBlock) {
    void* first = this->resource.allocate(TypeParam::kBlockSize, 1);
    this->resource.deallocate(first, TypeParam::kBlockSize, 1);

    void* second = this->resource.allocate(TypeParam::kBlockSize, 1);
    ASSERT_EQ(first, second); // Should reuse the same block
    this->resource.deallocate(second, TypeParam::kBlockSize, 1);
}

TYPED_TEST(PoolMemoryResourceTest, ReuseMultipleContiguousBlocks) {
    constexpr std::size_t multi_block_size = TypeParam::kBlockSize * 3;
    const std::size_t blocks_needed = this->blocks_needed(multi_block_size);

    // Allocate multiple contiguous blocks
    void* first = this->resource.allocate(multi_block_size, 1);
    this->resource.deallocate(first, multi_block_size, 1);

    // Allocate the same size again - should reuse the same contiguous blocks
    void* second = this->resource.allocate(multi_block_size, 1);
    ASSERT_EQ(first, second); // Should reuse the same memory region

    // Should be able to allocate remaining blocks
    const std::size_t remaining_blocks = TypeParam::kNumBlocks - blocks_needed;
    for (std::size_t i = 0; i < remaining_blocks; ++i) {
        void* block = this->resource.allocate(TypeParam::kBlockSize, 1);
        this->resource.deallocate(block, TypeParam::kBlockSize, 1);
    }

    this->resource.deallocate(second, multi_block_size, 1);
}

TYPED_TEST(PoolMemoryResourceTest, ReuseFragmentedBlocks) {
    // Test that freed blocks can be reused for different allocation patterns

    // Allocate two multi-block allocations
    constexpr std::size_t size1 = TypeParam::kBlockSize * 2;
    constexpr std::size_t size2 = TypeParam::kBlockSize * 3;

    void* block1 = this->resource.allocate(size1, 1);
    void* block2 = this->resource.allocate(size2, 1);

    // Deallocate them
    this->resource.deallocate(block1, size1, 1);
    this->resource.deallocate(block2, size2, 1);

    // Now allocate a larger block that should fit in the freed space
    constexpr std::size_t larger_size = TypeParam::kBlockSize * 4;
    if (larger_size <= TypeParam::kPoolSize) {
        void* larger_block = this->resource.allocate(larger_size, 1);
        this->resource.deallocate(larger_block, larger_size, 1);
    }

    // Or allocate the original sizes again
    void* new_block1 = this->resource.allocate(size1, 1);
    void* new_block2 = this->resource.allocate(size2, 1);

    this->resource.deallocate(new_block1, size1, 1);
    this->resource.deallocate(new_block2, size2, 1);
}

TYPED_TEST(PoolMemoryResourceTest, MixedBlockAllocations) {
    // Mix single and multi-block allocations
    std::vector<std::pair<void*, std::size_t>> allocations;

    // Allocate various sizes
    allocations.emplace_back(this->resource.allocate(TypeParam::kBlockSize, 1),
                             TypeParam::kBlockSize);
    allocations.emplace_back(
        this->resource.allocate(TypeParam::kBlockSize * 2, 1),
        TypeParam::kBlockSize * 2);
    allocations.emplace_back(this->resource.allocate(TypeParam::kBlockSize, 1),
                             TypeParam::kBlockSize);
    allocations.emplace_back(
        this->resource.allocate(TypeParam::kBlockSize * 3, 1),
        TypeParam::kBlockSize * 3);

    // Deallocate in different order
    this->resource.deallocate(allocations[1].first, allocations[1].second, 1);
    this->resource.deallocate(allocations[0].first, allocations[0].second, 1);

    // Allocate new blocks - should reuse freed memory
    void* new_block1 = this->resource.allocate(TypeParam::kBlockSize * 2, 1);
    void* new_block2 = this->resource.allocate(TypeParam::kBlockSize, 1);

    // Clean up remaining allocations
    this->resource.deallocate(allocations[2].first, allocations[2].second, 1);
    this->resource.deallocate(allocations[3].first, allocations[3].second, 1);
    this->resource.deallocate(new_block1, TypeParam::kBlockSize * 2, 1);
    this->resource.deallocate(new_block2, TypeParam::kBlockSize, 1);
}

TYPED_TEST(PoolMemoryResourceTest, AlignmentWithMultipleBlocks) {
    // Test alignment with multi-block allocations
    constexpr std::size_t multi_block_size = TypeParam::kBlockSize * 2;

    for (std::size_t alignment : {1, 2, 4, 8, 16, 32, 64}) {
        if (alignment <= TypeParam::kBlockSize) {
            void* block = this->resource.allocate(multi_block_size, alignment);
            EXPECT_EQ(reinterpret_cast<uintptr_t>(block) % alignment, 0u)
                << "Alignment " << alignment
                << " not satisfied for multi-block allocation";
            this->resource.deallocate(block, multi_block_size, alignment);
        }
    }
}

TYPED_TEST(PoolMemoryResourceTest, ExhaustPoolWithMixedSizes) {
    // Fill the pool with mixed size allocations
    std::vector<std::pair<void*, std::size_t>> allocations;
    std::size_t total_used_blocks = 0;

    // Keep allocating until pool is exhausted
    while (true) {
        // Try different sizes
        std::size_t sizes[] = {TypeParam::kBlockSize, TypeParam::kBlockSize * 2,
                               TypeParam::kBlockSize * 3};
        bool allocated = false;

        for (std::size_t size : sizes) {
            std::size_t blocks_required = this->blocks_needed(size);
            if (total_used_blocks + blocks_required <= TypeParam::kNumBlocks) {
                try {
                    void* block = this->resource.allocate(size, 1);
                    allocations.emplace_back(block, size);
                    total_used_blocks += blocks_required;
                    allocated = true;
                    break;
                } catch (const std::bad_alloc&) {
                    // Try next size
                }
            }
        }

        if (!allocated) {
            break; // Pool is exhausted
        }
    }

    // Should not be able to allocate more
    EXPECT_THROW(this->resource.allocate(TypeParam::kBlockSize, 1),
                 std::bad_alloc);

    // Deallocate half of the allocations
    std::size_t half = allocations.size() / 2;
    for (std::size_t i = 0; i < half; ++i) {
        this->resource.deallocate(allocations[i].first, allocations[i].second,
                                  1);
    }

    // Should be able to allocate again with freed memory
    void* new_block = this->resource.allocate(TypeParam::kBlockSize, 1);
    this->resource.deallocate(new_block, TypeParam::kBlockSize, 1);

    // Clean up remaining allocations
    for (std::size_t i = half; i < allocations.size(); ++i) {
        this->resource.deallocate(allocations[i].first, allocations[i].second,
                                  1);
    }
}

// Edge case tests
class PoolMemoryResourceEdgeCases : public ::testing::Test {
protected:
    static constexpr std::size_t kDefaultPoolSize = 1024;
    static constexpr std::size_t kDefaultBlockSize = 64;
};

TEST_F(PoolMemoryResourceEdgeCases, ExactBlockSizeAllocation) {
    PoolMemoryResource<kDefaultPoolSize, kDefaultBlockSize> resource;

    // Allocate exactly one block
    void* block = resource.allocate(kDefaultBlockSize, 1);
    resource.deallocate(block, kDefaultBlockSize, 1);
}

TEST_F(PoolMemoryResourceEdgeCases, SlightlyLargerThanBlockSize) {
    PoolMemoryResource<kDefaultPoolSize, kDefaultBlockSize> resource;

    // Allocation slightly larger than one block should use two blocks
    void* block = resource.allocate(kDefaultBlockSize + 1, 1);

    // Should be able to allocate remaining blocks (total blocks - 2)
    const std::size_t remaining_blocks =
        (kDefaultPoolSize / kDefaultBlockSize) - 2;
    for (std::size_t i = 0; i < remaining_blocks; ++i) {
        void* b = resource.allocate(kDefaultBlockSize, 1);
        resource.deallocate(b, kDefaultBlockSize, 1);
    }

    resource.deallocate(block, kDefaultBlockSize + 1, 1);
}

TEST_F(PoolMemoryResourceEdgeCases, MaximumPossibleAllocation) {
    PoolMemoryResource<kDefaultPoolSize, kDefaultBlockSize> resource;

    // Allocate the entire pool as one chunk
    void* block = resource.allocate(kDefaultPoolSize, 1);

    // No more allocations possible
    EXPECT_THROW(resource.allocate(1, 1), std::bad_alloc);

    resource.deallocate(block, kDefaultPoolSize, 1);

    // Should be able to allocate again after deallocation
    void* block2 = resource.allocate(kDefaultPoolSize, 1);
    resource.deallocate(block2, kDefaultPoolSize, 1);
}

TEST_F(PoolMemoryResourceEdgeCases, ComplexReusePattern) {
    PoolMemoryResource<1024, 64> resource; // 16 blocks

    // Phase 1: Allocate various sizes
    void* a = resource.allocate(64, 1);  // 1 block
    void* b = resource.allocate(128, 1); // 2 blocks
    void* c = resource.allocate(192, 1); // 3 blocks
    void* d = resource.allocate(64, 1);  // 1 block

    // Phase 2: Free some allocations
    resource.deallocate(b, 128, 1); // Free 2 blocks
    resource.deallocate(d, 64, 1);  // Free 1 block

    // Phase 3: Allocate new sizes that should fit in freed space
    void* e =
        resource.allocate(192, 1); // 3 blocks - should fit where b was + d?
    void* f = resource.allocate(64, 1); // 1 block

    // Phase 4: Free remaining and verify complete reuse
    resource.deallocate(a, 64, 1);
    resource.deallocate(c, 192, 1);
    resource.deallocate(e, 192, 1);
    resource.deallocate(f, 64, 1);

    // Should be able to allocate the entire pool again
    void* full_pool = resource.allocate(1024, 1);
    resource.deallocate(full_pool, 1024, 1);
}

TEST_F(PoolMemoryResourceEdgeCases, AlignmentWithMultiBlock) {
    PoolMemoryResource<1024, 64> resource;

    // Test large alignment with multi-block allocation
    constexpr std::size_t size = 128; // 2 blocks
    constexpr std::size_t alignment = 64;

    void* block = resource.allocate(size, alignment);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(block) % alignment, 0u);

    resource.deallocate(block, size, alignment);
}

TEST_F(PoolMemoryResourceEdgeCases, ExceedPoolCapacityThrows) {
    PoolMemoryResource<256, 64> resource; // 4 blocks

    // Allocate all blocks
    void* b1 = resource.allocate(64, 1);
    void* b2 = resource.allocate(64, 1);
    void* b3 = resource.allocate(64, 1);
    void* b4 = resource.allocate(64, 1);

    // Next allocation should throw
    EXPECT_THROW(resource.allocate(64, 1), std::bad_alloc);

    resource.deallocate(b1, 64, 1);
    resource.deallocate(b2, 64, 1);
    resource.deallocate(b3, 64, 1);
    resource.deallocate(b4, 64, 1);
}

TEST_F(PoolMemoryResourceEdgeCases, RequestTooLargeThrows) {
    PoolMemoryResource<1024, 64> resource;

    // Request more than pool size
    EXPECT_THROW(resource.allocate(2048, 1), std::bad_alloc);

    // Request exactly pool size + 1
    EXPECT_THROW(resource.allocate(1025, 1), std::bad_alloc);
}

TEST_F(PoolMemoryResourceEdgeCases, ZeroByteAllocation) {
    PoolMemoryResource<1024, 64> resource;

    // Zero-byte allocation behavior is implementation defined
    // Test that it doesn't crash and can be deallocated
    void* block = resource.allocate(0, 1);
    resource.deallocate(block, 0, 1);
}

TEST_F(PoolMemoryResourceEdgeCases, SequentialAllocationDeallocation) {
    PoolMemoryResource<512, 64> resource; // 8 blocks

    // Repeatedly allocate and deallocate different patterns
    for (int i = 0; i < 10; ++i) {
        void* a = resource.allocate(64, 1);
        void* b = resource.allocate(128, 1);
        void* c = resource.allocate(192, 1);

        resource.deallocate(a, 64, 1);
        resource.deallocate(c, 192, 1);

        void* d = resource.allocate(256, 1);

        resource.deallocate(b, 128, 1);
        resource.deallocate(d, 256, 1);
    }

    // Pool should still be functional
    void* final_block = resource.allocate(512, 1);
    resource.deallocate(final_block, 512, 1);
}

} // namespace
} // namespace lib::pmr
