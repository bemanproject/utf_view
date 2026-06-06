// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan 2025 - 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef BEMAN_UTF_VIEW_FAKE_INPLACE_VECTOR_HPP
#define BEMAN_UTF_VIEW_FAKE_INPLACE_VECTOR_HPP

#include <beman/utf_view/config.hpp>

#if BEMAN_UTF_VIEW_USE_MODULES() && \
    !defined(BEMAN_UTF_VIEW_INCLUDED_FROM_INTERFACE_UNIT)

import beman.utf_view;

#else

#if !BEMAN_UTF_VIEW_USE_MODULES()
#include <algorithm>
#include <cstddef>
#include <initializer_list>
#endif

namespace beman::utf_view::detail {

template <typename T, std::size_t N>
class fake_inplace_vector {
public:
  using value_type = T;

  constexpr fake_inplace_vector() = default;
  constexpr fake_inplace_vector(std::initializer_list<T> init)
  : size_{init.size()}
  {
    std::ranges::copy(init, storage_);
  }

  constexpr std::size_t size() const {
    return size_;
  }
  static constexpr std::size_t max_size() {
    return N;
  }
  constexpr void clear() {
    size_ = 0;
  }
  constexpr void push_back(T t) {
    storage_[size_++] = t;
  }
  constexpr T operator[](std::size_t n) const {
    return storage_[n];
  }
  constexpr T* begin() {
    return &storage_[0];
  }
  constexpr T* end() {
    return &storage_[size_];
  }
  constexpr const T* begin() const {
    return &storage_[0];
  }
  constexpr const T* end() const {
    return &storage_[size_];
  }
  constexpr T* data() {
    return &storage_[0];
  }
  constexpr const T* data() const {
    return &storage_[0];
  }

private:
  T storage_[N];
  std::size_t size_{};
};

} // namespace beman::utf_view::detail

#endif // BEMAN_UTF_VIEW_USE_MODULES() &&
       // !defined(BEMAN_UTF_VIEW_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_UTF_VIEW_FAKE_INPLACE_VECTOR_HPP
