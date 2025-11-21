#pragma once

#include "forward_list.hpp"

namespace lib::pmr {

template <typename T>
ForwardList<T>::ForwardListIterator::ForwardListIterator() noexcept
    : current_(nullptr) {}

template <typename T>
ForwardList<T>::ForwardListIterator::ForwardListIterator(Node* node) noexcept
    : current_(node) {}

template <typename T>
inline __attribute__((always_inline)) bool
ForwardList<T>::ForwardListIterator::operator==(
    const ForwardList<T>::ForwardListIterator& other) const {
    return this->current_ == other.current_;
}

template <typename T>
inline __attribute__((always_inline)) bool
ForwardList<T>::ForwardListIterator::operator!=(
    const ForwardList<T>::ForwardListIterator& other) const {
    return this->current_ != other.current_;
}

template <typename T>
ForwardList<T>::ForwardListIterator&
ForwardList<T>::ForwardListIterator::operator++() {
    current_ = current_->next;
    return *this;
}

template <typename T>
ForwardList<T>::ForwardListIterator
ForwardList<T>::ForwardListIterator::operator++(int) {
    ForwardList<T>::ForwardListIterator tmp = *this;
    ++(*this);
    return tmp;
}

template <typename T>
inline __attribute__((always_inline))
ForwardList<T>::ForwardListIterator::reference
ForwardList<T>::ForwardListIterator::operator*() const {
    return current_->value;
}

template <typename T>
inline
    __attribute__((always_inline)) ForwardList<T>::ForwardListIterator::pointer
    ForwardList<T>::ForwardListIterator::operator->() const {
    return &current_->value;
}

} // namespace lib::pmr
