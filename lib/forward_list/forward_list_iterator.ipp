#pragma once

#include "forward_list.hpp"

namespace lib::pmr {

namespace {

template <typename T>
using iterator_type = ForwardList<T>::ForwardListIterator;

}

template <typename T>
inline __attribute__((always_inline)) bool
ForwardList<T>::ForwardListIterator::operator==(
    const iterator_type<T>& other) const {
    return this->current_ == other->current_;
}

template <typename T>
inline __attribute__((always_inline)) bool
ForwardList<T>::ForwardListIterator::operator!=(
    const iterator_type<T>& other) const {
    return this->current_ != other->current_;
}

template <typename T>
iterator_type<T>& ForwardList<T>::ForwardListIterator::operator++() {
    current_ = current_->next;
    return *this;
}

template <typename T>
iterator_type<T> ForwardList<T>::ForwardListIterator::operator++(int) {
    iterator_type<T> tmp = *this;
    ++(*this);
    return tmp;
}

template <typename T>
inline __attribute__((always_inline)) iterator_type<T>::reference_type
ForwardList<T>::ForwardListIterator::operator*() const {
    return *current_->value;
}

template <typename T>
inline __attribute__((always_inline)) iterator_type<T>::pointer_type
ForwardList<T>::ForwardListIterator::operator->() const {
    return current_->value;
}

} // namespace lib::pmr
