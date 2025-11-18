#pragma once

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory_resource>

namespace lib::pmr {

template <typename T>
class ForwardList {
private:
    struct Node {
        Node* next;
        T* value;
    };

    std::pmr::polymorphic_allocator<T> allocator_;
    Node* head_;
    std::size_t size_;

public:
    using allocator_type = std::pmr::polymorphic_allocator<T>;
    class ForwardListIterator {
        friend class ForwardList;

    private:
        Node* current_;

    public:
        using value_type = T;
        using reference_type = value_type&;
        using pointer_type = value_type*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        inline __attribute__((always_inline)) bool
        operator==(const ForwardListIterator& other) const;
        inline __attribute__((always_inline)) bool
        operator!=(const ForwardListIterator& other) const;

        ForwardListIterator& operator++();
        ForwardListIterator operator++(int);

        inline __attribute__((always_inline)) reference_type operator*() const;
        inline __attribute__((always_inline)) pointer_type operator->() const;
    };

    ForwardList(allocator_type allocator = std::pmr::get_default_resource());

    explicit ForwardList(
        std::size_t size,
        allocator_type allocator = std::pmr::get_default_resource());

    ForwardList(const std::initializer_list<T>& values,
                allocator_type allocator = std::pmr::get_default_resource());

    ForwardList(const ForwardList& other);
    ForwardList(ForwardList&& other) noexcept;
    ForwardList& operator=(const ForwardList& other);
    ForwardList& operator=(ForwardList&& other);

    inline __attribute__((always_inline)) ForwardListIterator
    Begin() const noexcept;

    inline __attribute__((always_inline)) ForwardListIterator
    End() const noexcept;

    inline __attribute__((always_inline)) const T& Front() const noexcept;
    inline __attribute__((always_inline)) T& Front() noexcept;
    inline __attribute__((always_inline)) bool IsEmpty() const noexcept;
    inline __attribute__((always_inline)) std::size_t Size() const noexcept;

    template <typename... Args>
    void EmplaceFront(Args&&... args);

    template <typename U>
    void PushFront(U&& value);

    void PopFront();

    template <typename... Args>
    void EmplaceAfter(ForwardListIterator pos, Args&&... args);

    template <typename U>
    void InsertAfter(ForwardListIterator pos, U&& value);

    void EraseAfter(ForwardListIterator pos);

    void Clear();

    ~ForwardList();

    template <typename U>
    friend void swap(ForwardList<U>& a, ForwardList<U>& b);
};

} // namespace lib::pmr

#include "forward_list.ipp"
#include "forward_list_iterator.ipp"
