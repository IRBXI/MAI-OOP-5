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
    };

    std::pmr::polymorphic_allocator<Node> node_allocator_;
    Node* head_;
    std::size_t size_;

public:
    using allocator_type = std::pmr::polymorphic_allocator<T>;

    class ForwardListIterator {
        friend class ForwardList;

    private:
        Node* current_;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
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

    ForwardList(std::pmr::memory_resource* mem_resource =
                    std::pmr::get_default_resource());

    ForwardList(const std::initializer_list<T>& values,
                std::pmr::memory_resource* mem_resource =
                    std::pmr::get_default_resource());

    ForwardList(const ForwardList& other,
                std::pmr::memory_resource* mem_resource =
                    std::pmr::get_default_resource());

    ForwardList(ForwardList&& other,
                std::pmr::memory_resource* = std::pmr::get_default_resource());

    ForwardList& operator=(const ForwardList& other);
    ForwardList& operator=(ForwardList&& other);

    inline __attribute__((always_inline)) ForwardListIterator
    begin() const noexcept;

    inline __attribute__((always_inline)) ForwardListIterator
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

    template <typename... Args>
    void EmplaceAfter(ForwardListIterator pos, Args&&... args);

    template <typename U>
    void InsertAfter(ForwardListIterator pos, U&& value);

    void EraseAfter(ForwardListIterator pos) noexcept;

    void Clear() noexcept;

    ~ForwardList() noexcept;

private:
    template <typename... Args>
    void EmplaceFirstNode(Args&&... args);
};

} // namespace lib::pmr

#include "forward_list.ipp"
#include "forward_list_iterator.ipp"
