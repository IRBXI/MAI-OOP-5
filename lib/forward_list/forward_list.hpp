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
        T value;

        template <typename... Args>
        Node(Args&&... args);
    };

    std::pmr::polymorphic_allocator<T> allocator_;
    Node* head_;
    std::size_t size_;

public:
    using allocator_type = std::pmr::polymorphic_allocator<T>;

    template <bool is_const>
    class ForwardListIterator {
        friend class ForwardList;

    private:
        using NodeType = std::conditional_t<is_const, const Node*, Node*>;
        NodeType current_;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = std::conditional_t<is_const, const T, T>;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        ForwardListIterator() noexcept;
        ForwardListIterator(Node* node) noexcept;

        inline __attribute__((always_inline)) bool
        operator==(const ForwardListIterator& other) const;
        inline __attribute__((always_inline)) bool
        operator!=(const ForwardListIterator& other) const;

        ForwardListIterator& operator++();
        ForwardListIterator operator++(int);

        inline __attribute__((always_inline)) reference operator*() const;
        inline __attribute__((always_inline)) pointer operator->() const;
    };

    ForwardList(allocator_type allocator = allocator_type());

    ForwardList(const std::initializer_list<T>& values,
                allocator_type allocator = allocator_type());

    ForwardList(const ForwardList& other,
                allocator_type allocator = allocator_type());

    ForwardList(ForwardList&& other,
                allocator_type allocator = allocator_type());

    ForwardList& operator=(const ForwardList& other);
    ForwardList& operator=(ForwardList&& other);

    inline __attribute__((always_inline)) ForwardListIterator<false>
    begin() noexcept;
    inline __attribute__((always_inline)) ForwardListIterator<true>
    begin() const noexcept;

    inline __attribute__((always_inline)) ForwardListIterator<false>
    end() noexcept;
    inline __attribute__((always_inline)) ForwardListIterator<true>
    end() const noexcept;

    inline __attribute__((always_inline)) const T& Front() const noexcept;
    inline __attribute__((always_inline)) T& Front() noexcept;
    inline __attribute__((always_inline)) bool IsEmpty() const noexcept;
    inline __attribute__((always_inline)) std::size_t Size() const noexcept;

    template <typename... Args>
    void EmplaceFront(Args&&... args);

    template <typename U>
    void PushFront(U&& value);

    void PopFront() noexcept;

    template <typename... Args, bool is_const>
    void EmplaceAfter(ForwardListIterator<is_const> pos, Args&&... args);

    template <typename U, bool is_const>
    void InsertAfter(ForwardListIterator<is_const> pos, U&& value);

    template <bool is_const>
    void EraseAfter(ForwardListIterator<is_const> pos) noexcept;

    void Clear() noexcept;

    ~ForwardList() noexcept;

private:
    template <typename... Args>
    void EmplaceFirstNode(Args&&... args);
};

} // namespace lib::pmr

#include "forward_list.ipp"
#include "forward_list_iterator.ipp"
