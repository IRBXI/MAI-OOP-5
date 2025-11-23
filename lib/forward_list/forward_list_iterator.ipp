#pragma once

#include "forward_list.hpp"

namespace lib::pmr {

template <typename T>
template <bool is_const>
ForwardList<T>::ForwardListIterator<is_const>::ForwardListIterator() noexcept
    : current_(nullptr) {}

template <typename T>
template <bool is_const>
ForwardList<T>::ForwardListIterator<is_const>::ForwardListIterator(
    Node* node) noexcept
    : current_(node) {}

template <typename T>
template <bool is_const>
inline __attribute__((always_inline)) bool
ForwardList<T>::ForwardListIterator<is_const>::operator==(
    const ForwardList<T>::ForwardListIterator<is_const>& other) const {
    return this->current_ == other.current_;
}

template <typename T>
template <bool is_const>
inline __attribute__((always_inline)) bool
ForwardList<T>::ForwardListIterator<is_const>::operator!=(
    const ForwardList<T>::ForwardListIterator<is_const>& other) const {
    return this->current_ != other.current_;
}

template <typename T>
template <bool is_const>
ForwardList<T>::ForwardListIterator<is_const>&
ForwardList<T>::ForwardListIterator<is_const>::operator++() {
    current_ = current_->next;
    return *this;
}

template <typename T>
template <bool is_const>
ForwardList<T>::ForwardListIterator<is_const>
ForwardList<T>::ForwardListIterator<is_const>::operator++(int) {
    ForwardList<T>::ForwardListIterator tmp = *this;
    ++(*this);
    return tmp;
}

template <typename T>
template <bool is_const>
inline __attribute__((always_inline))
ForwardList<T>::ForwardListIterator<is_const>::reference
ForwardList<T>::ForwardListIterator<is_const>::operator*() const {
    return current_->value;
}

template <typename T>
template <bool is_const>
inline __attribute__((always_inline))
ForwardList<T>::ForwardListIterator<is_const>::pointer
ForwardList<T>::ForwardListIterator<is_const>::operator->() const {
    return &current_->value;
}

} // namespace lib::pmr
