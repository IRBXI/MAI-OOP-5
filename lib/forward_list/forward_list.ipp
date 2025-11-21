#pragma once

#include "forward_list.hpp"

#include <cassert>
#include <memory>
#include <ranges>

namespace lib::pmr {

template <typename T>
ForwardList<T>::ForwardList(std::pmr::memory_resource* mem_resource)
    : node_allocator_(mem_resource), head_(nullptr), size_(0) {}

template <typename T>
ForwardList<T>::ForwardList(const std::initializer_list<T>& values,
                            std::pmr::memory_resource* mem_resource)
    : node_allocator_(mem_resource), head_(nullptr), size_(0) {
    for (auto value : std::ranges::views::reverse(values)) {
        PushFront(value);
    }
}

template <typename T>
ForwardList<T>::ForwardList(const ForwardList<T>& other,
                            std::pmr::memory_resource* mem_resource)
    : node_allocator_(mem_resource), head_(nullptr), size_(0) {
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
ForwardList<T>::ForwardList(ForwardList<T>&& other,
                            std::pmr::memory_resource* mem_resource)
    : node_allocator_(mem_resource), head_(nullptr), size_(0) {
    if (other.IsEmpty()) {
        return;
    }
    // If they use the same allocator when we can actually move
    if (mem_resource->is_equal(*other.node_allocator_.resource())) {
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
            // If they use the same allocator when we can actually move
            if (node_allocator_.resource()->is_equal(
                    *other.node_allocator_.resource())) {
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
inline __attribute__((always_inline)) ForwardList<T>::ForwardListIterator
ForwardList<T>::begin() const noexcept {
    return ForwardListIterator(head_);
}

template <typename T>
inline __attribute__((always_inline)) ForwardList<T>::ForwardListIterator
ForwardList<T>::end() const noexcept {
    return ForwardListIterator(nullptr);
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
    if (!size_) {
        EmplaceFirstNode(std::forward<Args>(args)...);
        return;
    }
    Node* new_head = node_allocator_.allocate(1);
    std::construct_at(std::addressof(new_head->value),
                      std::forward<Args>(args)...);
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
    std::destroy_at(std::addressof(previous_head->value));
    node_allocator_.deallocate(previous_head, 1);
    size_--;
}

template <typename T>
template <typename... Args>
void ForwardList<T>::EmplaceAfter(ForwardListIterator pos, Args&&... args) {
    Node* next = pos.current_->next;
    Node* new_node = node_allocator_.allocate(1);
    std::construct_at(std::addressof(new_node->value),
                      std::forward<Args>(args)...);
    new_node->next = next;
    pos.current_->next = new_node;
    size_++;
}

template <typename T>
template <typename U>
void ForwardList<T>::InsertAfter(ForwardListIterator pos, U&& value) {
    EmplaceAfter(pos, std::forward<U>(value));
}

template <typename T>
void ForwardList<T>::EraseAfter(ForwardListIterator pos) noexcept {
    assert(size_ > 0);
    Node* next = pos.current_->next;
    pos.current_->next = next->next;
    std::destroy_at(std::addressof(next->value));
    node_allocator_.deallocate(next, 1);
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
    head_ = node_allocator_.allocate(1);
    std::construct_at(std::addressof(head_->value),
                      std::forward<Args>(args)...);
    head_->next = nullptr;
    size_++;
}

} // namespace lib::pmr
