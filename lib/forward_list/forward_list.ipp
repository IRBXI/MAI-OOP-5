#pragma once

#include "forward_list.hpp"

#include <cassert>
#include <ranges>

namespace lib::pmr {

template <typename T>
template <typename... Args>
ForwardList<T>::Node::Node(Args&&... args)
    : value(std::forward<Args>(args)...), next(nullptr) {}

template <typename T>
ForwardList<T>::ForwardList(allocator_type allocator)
    : allocator_(allocator), head_(nullptr), size_(0) {}

template <typename T>
ForwardList<T>::ForwardList(const std::initializer_list<T>& values,
                            allocator_type allocator)
    : allocator_(allocator), head_(nullptr), size_(0) {
    for (auto value : std::ranges::views::reverse(values)) {
        PushFront(value);
    }
}

template <typename T>
ForwardList<T>::ForwardList(const ForwardList<T>& other,
                            allocator_type allocator)
    : allocator_(allocator), head_(nullptr), size_(0) {
    if (other.IsEmpty()) {
        return;
    }
    EmplaceFirstNode(other.head_->value);
    auto it = begin();
    for (auto value : std::ranges::drop_view(other, 1)) {
        InsertAfter(it, value);
        it++;
    }
}

template <typename T>
ForwardList<T>::ForwardList(ForwardList<T>&& other, allocator_type allocator)
    : allocator_(allocator), head_(nullptr), size_(0) {
    if (other.IsEmpty()) {
        return;
    }
    // If they use the same allocator then we can actually move
    if (allocator_ == other.allocator_) {
        head_ = other.head_;
        size_ = other.size_;
        other.head_ = nullptr;
        other.size_ = 0;
        return;
    }
    // Otherwise copy
    EmplaceFirstNode(other.head_->value);
    auto it = begin();
    for (auto value : std::ranges::drop_view(other, 1)) {
        InsertAfter(it, value);
        it++;
    }
}

template <typename T>
ForwardList<T>& ForwardList<T>::operator=(const ForwardList& other) {
    if (this != &other) {
        Clear();
        if (!other.IsEmpty()) {
            EmplaceFirstNode(other.head_->value);
            auto it = begin();
            for (auto value : std::ranges::drop_view(other, 1)) {
                InsertAfter(it, value);
                it++;
            }
        }
    }
    return *this;
}

template <typename T>
ForwardList<T>& ForwardList<T>::operator=(ForwardList&& other) {
    if (this != &other) {
        Clear();
        if (!other.IsEmpty()) {
            // If they use the same allocator then we can actually move
            if (allocator_ == other.allocator_) {
                head_ = other.head_;
                size_ = other.size_;
                other.head_ = nullptr;
                other.size_ = 0;
                return *this;
            }
            // Otherwise copy
            EmplaceFirstNode(other.head_->value);
            auto it = begin();
            for (auto value : std::ranges::drop_view(other, 1)) {
                InsertAfter(it, value);
                it++;
            }
        }
    }
    return *this;
}

template <typename T>
inline __attribute__((always_inline)) ForwardList<T>::ForwardListIterator<false>
ForwardList<T>::begin() noexcept {
    return ForwardListIterator<false>(head_);
}

template <typename T>
inline __attribute__((always_inline)) ForwardList<T>::ForwardListIterator<true>
ForwardList<T>::begin() const noexcept {
    return ForwardListIterator<true>(head_);
}

template <typename T>
inline __attribute__((always_inline)) ForwardList<T>::ForwardListIterator<false>
ForwardList<T>::end() noexcept {
    return ForwardListIterator<false>(nullptr);
}

template <typename T>
inline __attribute__((always_inline)) ForwardList<T>::ForwardListIterator<true>
ForwardList<T>::end() const noexcept {
    return ForwardListIterator<true>(nullptr);
}

template <typename T>
inline __attribute__((always_inline)) const T&
ForwardList<T>::Front() const noexcept {
    assert(size_ > 0);
    return head_->value;
}

template <typename T>
inline __attribute__((always_inline)) T& ForwardList<T>::Front() noexcept {
    assert(size_ > 0);
    return head_->value;
}

template <typename T>
inline __attribute__((always_inline)) bool
ForwardList<T>::IsEmpty() const noexcept {
    return size_ == 0;
}

template <typename T>
inline __attribute__((always_inline)) size_t
ForwardList<T>::Size() const noexcept {
    return size_;
}

template <typename T>
template <typename... Args>
void ForwardList<T>::EmplaceFront(Args&&... args) {
    if (IsEmpty()) {
        EmplaceFirstNode(std::forward<Args>(args)...);
        return;
    }
    Node* new_head =
        allocator_.template new_object<Node>(std::forward<Args>(args)...);
    new_head->next = head_;
    head_ = new_head;
    size_++;
}

template <typename T>
template <typename U>
void ForwardList<T>::PushFront(U&& value) {
    EmplaceFront(std::forward<U>(value));
}

template <typename T>
void ForwardList<T>::PopFront() noexcept {
    assert(size_ > 0);
    Node* previous_head = head_;
    head_ = head_->next;
    allocator_.delete_object(previous_head);
    size_--;
}

template <typename T>
template <typename... Args, bool is_const>
void ForwardList<T>::EmplaceAfter(ForwardListIterator<is_const> pos,
                                  Args&&... args) {
    Node* next = pos.current_->next;
    Node* new_node =
        allocator_.template new_object<Node>(std::forward<Args>(args)...);
    new_node->next = next;
    pos.current_->next = new_node;
    size_++;
}

template <typename T>
template <typename U, bool is_const>
void ForwardList<T>::InsertAfter(ForwardListIterator<is_const> pos, U&& value) {
    EmplaceAfter(pos, std::forward<U>(value));
}

template <typename T>
template <bool is_const>
void ForwardList<T>::EraseAfter(ForwardListIterator<is_const> pos) noexcept {
    assert(size_ > 0);
    Node* next = pos.current_->next;
    pos.current_->next = next->next;
    allocator_.delete_object(next);
    size_--;
}

template <typename T>
void ForwardList<T>::Clear() noexcept {
    while (size_) {
        PopFront();
    }
}

template <typename T>
ForwardList<T>::~ForwardList() noexcept {
    Clear();
}

template <typename T>
template <typename... Args>
void ForwardList<T>::EmplaceFirstNode(Args&&... args) {
    head_ = allocator_.template new_object<Node>(std::forward<Args>(args)...);
    head_->next = nullptr;
    size_++;
}

} // namespace lib::pmr
