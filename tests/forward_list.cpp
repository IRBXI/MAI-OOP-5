#include <gtest/gtest.h>

#include "forward_list.hpp"
#include "pool_allocator.hpp"

namespace lib::pmr {
namespace {

// Test fixture for ForwardList tests
template <typename T>
class ForwardListTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup for each test
    }

    void TearDown() override {
        // Cleanup for each test
    }
};

// Types to test with - using only types that can be properly constructed
using TestTypes = ::testing::Types<int, double, std::vector<int>>;
TYPED_TEST_SUITE(ForwardListTest, TestTypes);

// Special test fixture for std::string since it requires different construction
class ForwardListStringTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

// Test fixture for allocator-specific tests
class ForwardListAllocatorTest : public ::testing::Test {
protected:
    static constexpr std::size_t kPoolSize = 4096;
    static constexpr std::size_t kBlockSize = 64;

    void SetUp() override {
        pool_resource_ =
            std::make_unique<PoolMemoryResource<kPoolSize, kBlockSize>>();
    }

    std::unique_ptr<PoolMemoryResource<kPoolSize, kBlockSize>> pool_resource_;
};

// Basic construction and destruction tests
TYPED_TEST(ForwardListTest, DefaultConstructor) {
    ForwardList<TypeParam> list;
    EXPECT_TRUE(list.IsEmpty());
    EXPECT_EQ(list.Size(), 0);
    EXPECT_EQ(list.begin(), list.end());
}

TYPED_TEST(ForwardListTest, InitializerListConstructor) {
    ForwardList<TypeParam> list{TypeParam(1), TypeParam(2), TypeParam(3)};
    EXPECT_FALSE(list.IsEmpty());
    EXPECT_EQ(list.Size(), 3);

    auto it = list.begin();
    EXPECT_EQ(*it, TypeParam(1));
    ++it;
    EXPECT_EQ(*it, TypeParam(2));
    ++it;
    EXPECT_EQ(*it, TypeParam(3));
    ++it;
    EXPECT_EQ(it, list.end());
}

TYPED_TEST(ForwardListTest, CopyConstructor) {
    ForwardList<TypeParam> original{TypeParam(1), TypeParam(2), TypeParam(3)};
    ForwardList<TypeParam> copy(original);

    EXPECT_EQ(original.Size(), copy.Size());
    EXPECT_FALSE(copy.IsEmpty());

    auto orig_it = original.begin();
    auto copy_it = copy.begin();
    while (orig_it != original.end() && copy_it != copy.end()) {
        EXPECT_EQ(*orig_it, *copy_it);
        ++orig_it;
        ++copy_it;
    }
}

TYPED_TEST(ForwardListTest, CopyAssignment) {
    ForwardList<TypeParam> original{TypeParam(1), TypeParam(2), TypeParam(3)};
    ForwardList<TypeParam> copy;

    copy = original;

    EXPECT_EQ(original.Size(), copy.Size());

    auto orig_it = original.begin();
    auto copy_it = copy.begin();
    while (orig_it != original.end() && copy_it != copy.end()) {
        EXPECT_EQ(*orig_it, *copy_it);
        ++orig_it;
        ++copy_it;
    }
}

TYPED_TEST(ForwardListTest, SelfAssignment) {
    ForwardList<TypeParam> list{TypeParam(1), TypeParam(2), TypeParam(3)};
    std::size_t original_size = list.Size();

    // Create a reference to avoid clang-tidy warning about self-assignment
    ForwardList<TypeParam>& list_ref = list;
    list = list_ref; // Self-assignment

    EXPECT_EQ(list.Size(), original_size);

    auto it = list.begin();
    EXPECT_EQ(*it, TypeParam(1));
    ++it;
    EXPECT_EQ(*it, TypeParam(2));
    ++it;
    EXPECT_EQ(*it, TypeParam(3));
}

// Iterator tests
TYPED_TEST(ForwardListTest, IteratorOperations) {
    ForwardList<TypeParam> list{TypeParam(1), TypeParam(2), TypeParam(3)};

    // Test equality/inequality
    auto it1 = list.begin();
    auto it2 = list.begin();
    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 != it2);

    // Test pre-increment
    ++it1;
    EXPECT_FALSE(it1 == it2);
    EXPECT_TRUE(it1 != it2);
    EXPECT_EQ(*it1, TypeParam(2));

    // Test post-increment
    auto it3 = it2++;
    EXPECT_EQ(*it3, TypeParam(1));
    EXPECT_EQ(*it2, TypeParam(2));

    // Test dereference and arrow (for types that support it)
    it1 = list.begin();
    *it1 = TypeParam(10);
    EXPECT_EQ(*it1, TypeParam(10));
}

TYPED_TEST(ForwardListTest, IteratorRange) {
    ForwardList<TypeParam> list{TypeParam(1), TypeParam(2), TypeParam(3)};

    std::vector<TypeParam> values;
    for (auto it = list.begin(); it != list.end(); ++it) {
        values.push_back(*it);
    }

    EXPECT_EQ(values.size(), 3u);
    EXPECT_EQ(values[0], TypeParam(1));
    EXPECT_EQ(values[1], TypeParam(2));
    EXPECT_EQ(values[2], TypeParam(3));
}

TYPED_TEST(ForwardListTest, ConstIterator) {
    const ForwardList<TypeParam> list{TypeParam(1), TypeParam(2), TypeParam(3)};

    std::vector<TypeParam> values;
    for (auto it = list.begin(); it != list.end(); ++it) {
        values.push_back(*it);
    }

    EXPECT_EQ(values.size(), 3u);
}

// Accessor tests
TYPED_TEST(ForwardListTest, FrontAccessors) {
    ForwardList<TypeParam> list{TypeParam(1), TypeParam(2), TypeParam(3)};

    EXPECT_EQ(list.Front(), TypeParam(1));

    // Test non-const version
    list.Front() = TypeParam(10);
    EXPECT_EQ(list.Front(), TypeParam(10));

    // Test const version
    const auto& const_list = list;
    EXPECT_EQ(const_list.Front(), TypeParam(10));
}

TYPED_TEST(ForwardListTest, EmptyAndSize) {
    ForwardList<TypeParam> list;
    EXPECT_TRUE(list.IsEmpty());
    EXPECT_EQ(list.Size(), 0);

    list.PushFront(TypeParam(1));
    EXPECT_FALSE(list.IsEmpty());
    EXPECT_EQ(list.Size(), 1);

    list.PopFront();
    EXPECT_TRUE(list.IsEmpty());
    EXPECT_EQ(list.Size(), 0);
}

// Modifier tests - Front operations
TYPED_TEST(ForwardListTest, PushFront) {
    ForwardList<TypeParam> list;

    list.PushFront(TypeParam(3));
    EXPECT_EQ(list.Front(), TypeParam(3));
    EXPECT_EQ(list.Size(), 1);

    list.PushFront(TypeParam(2));
    EXPECT_EQ(list.Front(), TypeParam(2));
    EXPECT_EQ(list.Size(), 2);

    list.PushFront(TypeParam(1));
    EXPECT_EQ(list.Front(), TypeParam(1));
    EXPECT_EQ(list.Size(), 3);

    // Verify order
    auto it = list.begin();
    EXPECT_EQ(*it, TypeParam(1));
    ++it;
    EXPECT_EQ(*it, TypeParam(2));
    ++it;
    EXPECT_EQ(*it, TypeParam(3));
}

TYPED_TEST(ForwardListTest, EmplaceFront) {
    // Use a simple struct for emplace testing
    struct TestStruct {
        TypeParam a;
        TypeParam b;
        bool operator==(const TestStruct& other) const {
            return a == other.a && b == other.b;
        }
    };

    ForwardList<TestStruct> list;

    list.EmplaceFront(TypeParam(1), TypeParam(2));
    EXPECT_EQ(list.Front(), (TestStruct{TypeParam(1), TypeParam(2)}));
    EXPECT_EQ(list.Size(), 1);

    list.EmplaceFront(TypeParam(3), TypeParam(4));
    EXPECT_EQ(list.Front(), (TestStruct{TypeParam(3), TypeParam(4)}));
    EXPECT_EQ(list.Size(), 2);
}

TYPED_TEST(ForwardListTest, PopFront) {
    ForwardList<TypeParam> list{TypeParam(1), TypeParam(2), TypeParam(3)};

    list.PopFront();
    EXPECT_EQ(list.Front(), TypeParam(2));
    EXPECT_EQ(list.Size(), 2);

    list.PopFront();
    EXPECT_EQ(list.Front(), TypeParam(3));
    EXPECT_EQ(list.Size(), 1);

    list.PopFront();
    EXPECT_TRUE(list.IsEmpty());
    EXPECT_EQ(list.Size(), 0);
}

// Modifier tests - After operations
TYPED_TEST(ForwardListTest, InsertAfter) {
    ForwardList<TypeParam> list{TypeParam(1), TypeParam(3)};

    auto it = list.begin(); // Points to 1
    list.InsertAfter(it, TypeParam(2));

    EXPECT_EQ(list.Size(), 3);
    auto check_it = list.begin();
    EXPECT_EQ(*check_it, TypeParam(1));
    ++check_it;
    EXPECT_EQ(*check_it, TypeParam(2));
    ++check_it;
    EXPECT_EQ(*check_it, TypeParam(3));
}

TYPED_TEST(ForwardListTest, EmplaceAfter) {
    // Use a simple struct for emplace testing
    struct TestStruct {
        TypeParam a;
        TypeParam b;
        bool operator==(const TestStruct& other) const {
            return a == other.a && b == other.b;
        }
    };

    ForwardList<TestStruct> list;
    list.EmplaceFront(TypeParam(1), TypeParam(1));
    list.EmplaceFront(TypeParam(3), TypeParam(3));

    auto it = list.begin(); // Points to (3,3)
    list.EmplaceAfter(it, TypeParam(2), TypeParam(2));

    EXPECT_EQ(list.Size(), 3);
    auto check_it = list.begin();
    EXPECT_EQ(*check_it, (TestStruct{TypeParam(3), TypeParam(3)}));
    ++check_it;
    EXPECT_EQ(*check_it, (TestStruct{TypeParam(2), TypeParam(2)}));
    ++check_it;
    EXPECT_EQ(*check_it, (TestStruct{TypeParam(1), TypeParam(1)}));
}

TYPED_TEST(ForwardListTest, EraseAfter) {
    ForwardList<TypeParam> list{TypeParam(1), TypeParam(2), TypeParam(3),
                                TypeParam(4)};

    auto it = list.begin(); // Points to 1
    list.EraseAfter(it);    // Erase 2

    EXPECT_EQ(list.Size(), 3);
    auto check_it = list.begin();
    EXPECT_EQ(*check_it, TypeParam(1));
    ++check_it;
    EXPECT_EQ(*check_it, TypeParam(3)); // 2 was erased
    ++check_it;
    EXPECT_EQ(*check_it, TypeParam(4));

    // Erase after the new second element (erase 4)
    it = list.begin();
    ++it; // Points to 3
    list.EraseAfter(it);

    EXPECT_EQ(list.Size(), 2);
    check_it = list.begin();
    EXPECT_EQ(*check_it, TypeParam(1));
    ++check_it;
    EXPECT_EQ(*check_it, TypeParam(3));
}

TYPED_TEST(ForwardListTest, EraseAfterLastElement) {
    ForwardList<TypeParam> list{TypeParam(1), TypeParam(2)};

    auto it = list.begin(); // Points to 1
    list.EraseAfter(it);    // Erase 2

    EXPECT_EQ(list.Size(), 1);
    EXPECT_EQ(list.Front(), TypeParam(1));
}

// Clear and destructor tests
TYPED_TEST(ForwardListTest, Clear) {
    ForwardList<TypeParam> list{TypeParam(1), TypeParam(2), TypeParam(3)};

    EXPECT_FALSE(list.IsEmpty());
    list.Clear();
    EXPECT_TRUE(list.IsEmpty());
    EXPECT_EQ(list.Size(), 0);
    EXPECT_EQ(list.begin(), list.end());

    // Clear empty list
    list.Clear();
    EXPECT_TRUE(list.IsEmpty());
}

TYPED_TEST(ForwardListTest, Destructor) {
    // This is mostly to ensure no memory leaks
    {
        ForwardList<TypeParam> list{TypeParam(1), TypeParam(2), TypeParam(3)};
        // list goes out of scope here, destructor should clean up
    }
    // If we get here without crashing, destructor probably worked
    SUCCEED();
}

// Tests with custom allocator
TEST_F(ForwardListAllocatorTest, ConstructionWithCustomAllocator) {
    ForwardList<int> list(pool_resource_.get());
    EXPECT_TRUE(list.IsEmpty());

    list.PushFront(1);
    list.PushFront(2);
    list.PushFront(3);

    EXPECT_EQ(list.Size(), 3);
    EXPECT_EQ(list.Front(), 3);
}

TEST_F(ForwardListAllocatorTest, InitializerListWithCustomAllocator) {
    ForwardList<int> list({1, 2, 3}, pool_resource_.get());

    EXPECT_EQ(list.Size(), 3);
    auto it = list.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 3);
}

TEST_F(ForwardListAllocatorTest, CopyConstructorWithCustomAllocator) {
    ForwardList<int> original({1, 2, 3}, pool_resource_.get());
    ForwardList<int> copy(original);

    EXPECT_EQ(original.Size(), copy.Size());
    auto orig_it = original.begin();
    auto copy_it = copy.begin();
    while (orig_it != original.end()) {
        EXPECT_EQ(*orig_it, *copy_it);
        ++orig_it;
        ++copy_it;
    }
}

TEST_F(ForwardListAllocatorTest, CopyConstructorWithDifferentAllocator) {
    // Create original with pool allocator
    ForwardList<int> original({1, 2, 3}, pool_resource_.get());

    // Create copy with default allocator
    ForwardList<int> copy(original, std::pmr::get_default_resource());

    EXPECT_EQ(original.Size(), copy.Size());
    auto orig_it = original.begin();
    auto copy_it = copy.begin();
    while (orig_it != original.end()) {
        EXPECT_EQ(*orig_it, *copy_it);
        ++orig_it;
        ++copy_it;
    }
}

TEST_F(ForwardListAllocatorTest, CopyAssignmentWithCustomAllocator) {
    ForwardList<int> original({1, 2, 3}, pool_resource_.get());
    ForwardList<int> copy(pool_resource_.get());

    copy = original;

    EXPECT_EQ(original.Size(), copy.Size());
    auto orig_it = original.begin();
    auto copy_it = copy.begin();
    while (orig_it != original.end()) {
        EXPECT_EQ(*orig_it, *copy_it);
        ++orig_it;
        ++copy_it;
    }
}

TEST_F(ForwardListAllocatorTest, LargeListWithPoolAllocator) {
    // Test that the pool allocator is actually being used
    // by creating a large list that would benefit from pool allocation
    ForwardList<int> list(pool_resource_.get());

    const int num_elements = 64;
    for (int i = 0; i < num_elements; ++i) {
        list.PushFront(i);
    }

    EXPECT_EQ(list.Size(), num_elements);

    // Verify elements are in reverse order (since we used PushFront)
    int expected = num_elements - 1;
    for (auto it = list.begin(); it != list.end(); ++it) {
        EXPECT_EQ(*it, expected);
        --expected;
    }
}

// String-specific tests
TEST_F(ForwardListStringTest, StringOperations) {
    ForwardList<std::string> list;

    // Use proper string construction
    list.PushFront("third");
    list.PushFront("second");
    list.PushFront("first");

    EXPECT_EQ(list.Size(), 3);
    EXPECT_EQ(list.Front(), "first");

    auto it = list.begin();
    EXPECT_EQ(*it, "first");
    ++it;
    EXPECT_EQ(*it, "second");
    ++it;
    EXPECT_EQ(*it, "third");
}

TEST_F(ForwardListStringTest, StringEmplace) {
    ForwardList<std::string> list;

    // Emplace with string literals
    list.EmplaceFront("world");
    list.EmplaceFront("hello");

    EXPECT_EQ(list.Size(), 2);
    EXPECT_EQ(list.Front(), "hello");

    auto it = list.begin();
    list.EmplaceAfter(it, "beautiful");

    EXPECT_EQ(list.Size(), 3);
    it = list.begin();
    EXPECT_EQ(*it, "hello");
    ++it;
    EXPECT_EQ(*it, "beautiful");
    ++it;
    EXPECT_EQ(*it, "world");
}

TEST_F(ForwardListStringTest, StringInitializerList) {
    // Use proper string initializer list
    ForwardList<std::string> list{"apple", "banana", "cherry"};

    EXPECT_EQ(list.Size(), 3);
    auto it = list.begin();
    EXPECT_EQ(*it, "apple");
    ++it;
    EXPECT_EQ(*it, "banana");
    ++it;
    EXPECT_EQ(*it, "cherry");
}

TEST_F(ForwardListStringTest, StringCopyOperations) {
    ForwardList<std::string> original{"one", "two", "three"};
    ForwardList<std::string> copy(original);

    EXPECT_EQ(original.Size(), copy.Size());
    auto orig_it = original.begin();
    auto copy_it = copy.begin();
    while (orig_it != original.end()) {
        EXPECT_EQ(*orig_it, *copy_it);
        ++orig_it;
        ++copy_it;
    }
}

TEST_F(ForwardListAllocatorTest, StringWithPoolAllocator) {
    ForwardList<std::string> list(pool_resource_.get());

    // Test string operations with pool allocator
    list.PushFront("third");
    list.EmplaceFront("second");
    list.PushFront("first");

    EXPECT_EQ(list.Size(), 3);
    EXPECT_EQ(list.Front(), "first");

    list.PopFront();
    EXPECT_EQ(list.Front(), "second");

    auto it = list.begin();
    list.InsertAfter(it, "inserted");

    EXPECT_EQ(list.Size(), 3);
    it = list.begin();
    EXPECT_EQ(*it, "second");
    ++it;
    EXPECT_EQ(*it, "inserted");
    ++it;
    EXPECT_EQ(*it, "third");
}

// Edge case tests
TEST_F(ForwardListAllocatorTest, SingleElementList) {
    ForwardList<int> list(pool_resource_.get());
    list.PushFront(42);

    EXPECT_EQ(list.Size(), 1);
    EXPECT_EQ(list.Front(), 42);
    EXPECT_FALSE(list.IsEmpty());

    auto it = list.begin();
    EXPECT_EQ(*it, 42);
    ++it;
    EXPECT_EQ(it, list.end());

    list.PopFront();
    EXPECT_TRUE(list.IsEmpty());
}

TEST_F(ForwardListAllocatorTest, InsertAfterSingleElement) {
    ForwardList<int> list(pool_resource_.get());
    list.PushFront(1);

    auto it = list.begin();
    list.InsertAfter(it, 2);

    EXPECT_EQ(list.Size(), 2);
    EXPECT_EQ(list.Front(), 1);

    it = list.begin();
    ++it;
    EXPECT_EQ(*it, 2);
}

TEST_F(ForwardListAllocatorTest, ChainedOperations) {
    ForwardList<int> list(pool_resource_.get());

    // Chain multiple operations
    list.PushFront(1);
    list.EmplaceFront(2);
    auto it = list.begin();
    list.InsertAfter(it, 3);
    list.EmplaceAfter(it, 4);
    list.PopFront();
    list.EraseAfter(list.begin());

    EXPECT_EQ(list.Size(), 2);
    auto check_it = list.begin();
    EXPECT_EQ(*check_it, 4);
    ++check_it;
    EXPECT_EQ(*check_it, 1);
}

// Test fixture for move operations with different allocator scenarios
class ForwardListMoveTest : public ::testing::Test {
protected:
    static constexpr std::size_t kPoolSize = 4096;
    static constexpr std::size_t kBlockSize = 64;

    void SetUp() override {
        pool_resource1_ =
            std::make_unique<PoolMemoryResource<kPoolSize, kBlockSize>>();
        pool_resource2_ =
            std::make_unique<PoolMemoryResource<kPoolSize, kBlockSize>>();
    }

    std::unique_ptr<PoolMemoryResource<kPoolSize, kBlockSize>> pool_resource1_;
    std::unique_ptr<PoolMemoryResource<kPoolSize, kBlockSize>> pool_resource2_;
};

// Move constructor tests
TEST_F(ForwardListMoveTest, MoveConstructorSameMemoryResource) {
    ForwardList<int> original({1, 2, 3}, pool_resource1_.get());
    std::size_t original_size = original.Size();
    void* original_first_element = &original.Front();

    // Move construct with same memory resource - should actually move
    ForwardList<int> moved(std::move(original), pool_resource1_.get());

    EXPECT_EQ(moved.Size(), original_size);
    EXPECT_TRUE(original.IsEmpty()); // Original should be empty after move

    // Verify elements were moved (not copied)
    auto it = moved.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 3);

    // The first element should be at the same memory location if moved
    EXPECT_EQ(&moved.Front(), original_first_element);
}

TEST_F(ForwardListMoveTest, MoveConstructorDifferentMemoryResource) {
    ForwardList<int> original({1, 2, 3}, pool_resource1_.get());
    std::size_t original_size = original.Size();

    // Move construct with different memory resource - should copy instead of
    // move
    ForwardList<int> moved(std::move(original), pool_resource2_.get());

    EXPECT_EQ(moved.Size(), original_size);
    EXPECT_EQ(original.Size(), original_size); // Original should remain intact

    // Verify elements were copied
    auto moved_it = moved.begin();
    auto original_it = original.begin();
    while (moved_it != moved.end() && original_it != original.end()) {
        EXPECT_EQ(*moved_it, *original_it);
        ++moved_it;
        ++original_it;
    }
}

TEST_F(ForwardListMoveTest, MoveConstructorDefaultResourceToPool) {
    ForwardList<int> original({1, 2, 3}, std::pmr::get_default_resource());
    std::size_t original_size = original.Size();

    // Move from default resource to pool resource - should copy
    ForwardList<int> moved(std::move(original), pool_resource1_.get());

    EXPECT_EQ(moved.Size(), original_size);
    EXPECT_EQ(original.Size(), original_size); // Original should remain intact

    // Verify elements were copied
    auto moved_it = moved.begin();
    auto original_it = original.begin();
    while (moved_it != moved.end() && original_it != original.end()) {
        EXPECT_EQ(*moved_it, *original_it);
        ++moved_it;
        ++original_it;
    }
}

TEST_F(ForwardListMoveTest, MoveConstructorPoolToDefaultResource) {
    ForwardList<int> original({1, 2, 3}, pool_resource1_.get());
    std::size_t original_size = original.Size();

    // Move from pool resource to default resource - should copy
    ForwardList<int> moved(std::move(original),
                           std::pmr::get_default_resource());

    EXPECT_EQ(moved.Size(), original_size);
    EXPECT_EQ(original.Size(), original_size); // Original should remain intact

    // Verify elements were copied
    auto moved_it = moved.begin();
    auto original_it = original.begin();
    while (moved_it != moved.end() && original_it != original.end()) {
        EXPECT_EQ(*moved_it, *original_it);
        ++moved_it;
        ++original_it;
    }
}

TEST_F(ForwardListMoveTest, MoveConstructorEmptyList) {
    ForwardList<int> original(pool_resource1_.get());
    EXPECT_TRUE(original.IsEmpty());

    // Move construct empty list with same memory resource
    ForwardList<int> moved(std::move(original), pool_resource1_.get());

    EXPECT_TRUE(moved.IsEmpty());
    EXPECT_TRUE(original.IsEmpty()); // Original should still be empty
}

// Move assignment operator tests
TEST_F(ForwardListMoveTest, MoveAssignmentSameMemoryResource) {
    ForwardList<int> original({1, 2, 3}, pool_resource1_.get());
    ForwardList<int> target({4, 5}, pool_resource1_.get());
    std::size_t original_size = original.Size();
    void* original_first_element = &original.Front();

    // Move assign with same memory resource - should actually move
    target = std::move(original);

    EXPECT_EQ(target.Size(), original_size);
    EXPECT_TRUE(original.IsEmpty()); // Original should be empty after move

    // Verify elements were moved (not copied)
    auto it = target.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 3);

    // The first element should be at the same memory location if moved
    EXPECT_EQ(&target.Front(), original_first_element);
}

TEST_F(ForwardListMoveTest, MoveAssignmentDifferentMemoryResource) {
    ForwardList<int> original({1, 2, 3}, pool_resource1_.get());
    ForwardList<int> target({4, 5}, pool_resource2_.get());
    std::size_t original_size = original.Size();

    // Move assign with different memory resource - should copy instead of move
    target = std::move(original);

    EXPECT_EQ(target.Size(), original_size);
    EXPECT_EQ(original.Size(), original_size); // Original should remain intact

    // Verify elements were copied
    auto target_it = target.begin();
    auto original_it = original.begin();
    while (target_it != target.end() && original_it != original.end()) {
        EXPECT_EQ(*target_it, *original_it);
        ++target_it;
        ++original_it;
    }
}

TEST_F(ForwardListMoveTest, MoveAssignmentDefaultToPoolResource) {
    ForwardList<int> original({1, 2, 3}, std::pmr::get_default_resource());
    ForwardList<int> target(pool_resource1_.get());
    std::size_t original_size = original.Size();

    // Move from default to pool resource - should copy
    target = std::move(original);

    EXPECT_EQ(target.Size(), original_size);
    EXPECT_EQ(original.Size(), original_size); // Original should remain intact

    // Verify elements were copied
    auto target_it = target.begin();
    auto original_it = original.begin();
    while (target_it != target.end() && original_it != original.end()) {
        EXPECT_EQ(*target_it, *original_it);
        ++target_it;
        ++original_it;
    }
}

TEST_F(ForwardListMoveTest, MoveAssignmentPoolToDefaultResource) {
    ForwardList<int> original({1, 2, 3}, pool_resource1_.get());
    ForwardList<int> target(std::pmr::get_default_resource());
    std::size_t original_size = original.Size();

    // Move from pool to default resource - should copy
    target = std::move(original);

    EXPECT_EQ(target.Size(), original_size);
    EXPECT_EQ(original.Size(), original_size); // Original should remain intact

    // Verify elements were copied
    auto target_it = target.begin();
    auto original_it = original.begin();
    while (target_it != target.end() && original_it != original.end()) {
        EXPECT_EQ(*target_it, *original_it);
        ++target_it;
        ++original_it;
    }
}

TEST_F(ForwardListMoveTest, MoveAssignmentEmptyToNonEmpty) {
    ForwardList<int> original(pool_resource1_.get()); // Empty
    ForwardList<int> target({1, 2, 3}, pool_resource1_.get());

    // Move assign empty to non-empty with same memory resource
    target = std::move(original);

    EXPECT_TRUE(target.IsEmpty());
    EXPECT_TRUE(original.IsEmpty()); // Original should still be empty
}

TEST_F(ForwardListMoveTest, MoveAssignmentNonEmptyToEmpty) {
    ForwardList<int> original({1, 2, 3}, pool_resource1_.get());
    ForwardList<int> target(pool_resource1_.get()); // Empty

    // Move assign non-empty to empty with same memory resource
    target = std::move(original);

    EXPECT_EQ(target.Size(), 3);
    EXPECT_TRUE(original.IsEmpty()); // Original should be empty after move

    auto it = target.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 3);
}

TEST_F(ForwardListMoveTest, MoveAssignmentSelf) {
    ForwardList<int> list({1, 2, 3}, pool_resource1_.get());
    std::size_t original_size = list.Size();

    // Self move assignment
    list = std::move(list);

    // Should remain unchanged
    EXPECT_EQ(list.Size(), original_size);

    auto it = list.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 3);
}

TEST_F(ForwardListMoveTest, MoveOperationsWithStrings) {
    // Test move operations with std::string to ensure they work with
    // non-trivial types
    ForwardList<std::string> original({"hello", "world", "test"},
                                      pool_resource1_.get());
    std::size_t original_size = original.Size();

    // Move constructor with same memory resource
    ForwardList<std::string> moved(std::move(original), pool_resource1_.get());

    EXPECT_EQ(moved.Size(), original_size);
    EXPECT_TRUE(original.IsEmpty()); // Should actually move with same resource

    auto it = moved.begin();
    EXPECT_EQ(*it, "hello");
    ++it;
    EXPECT_EQ(*it, "world");
    ++it;
    EXPECT_EQ(*it, "test");
}

TEST_F(ForwardListMoveTest, ComplexMoveScenario) {
    // Create a complex scenario with multiple lists and operations
    ForwardList<int> source(pool_resource1_.get());
    for (int i = 0; i < 10; ++i) {
        source.PushFront(i);
    }

    std::size_t source_size = source.Size();

    // Chain of move operations with same allocator
    ForwardList<int> intermediate1(std::move(source), pool_resource1_.get());
    ForwardList<int> intermediate2(pool_resource1_.get());
    intermediate2 = std::move(intermediate1);
    ForwardList<int> final(std::move(intermediate2), pool_resource1_.get());

    EXPECT_EQ(final.Size(), source_size);
    EXPECT_TRUE(source.IsEmpty());
    EXPECT_TRUE(intermediate1.IsEmpty());
    EXPECT_TRUE(intermediate2.IsEmpty());

    // Verify final list has correct elements in reverse order (due to
    // PushFront)
    int expected = 9;
    for (auto it = final.begin(); it != final.end(); ++it) {
        EXPECT_EQ(*it, expected);
        --expected;
    }
}

TEST_F(ForwardListMoveTest, MoveThenModify) {
    // Test that after move, we can still modify the moved-to list
    ForwardList<int> original({1, 2, 3}, pool_resource1_.get());

    ForwardList<int> moved(std::move(original), pool_resource1_.get());

    // Modify the moved list
    moved.PushFront(0);
    moved.PopFront();
    moved.InsertAfter(moved.begin(), 99);

    EXPECT_EQ(moved.Size(), 4);
    auto it = moved.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 99);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 3);
}

} // namespace
} // namespace lib::pmr
