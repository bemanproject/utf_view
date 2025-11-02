// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef BEMAN_UTF_VIEW_FAKE_INPLACE_VECTOR_HPP
#define BEMAN_UTF_VIEW_FAKE_INPLACE_VECTOR_HPP

#include <algorithm>
#include <cstddef>
#include <initializer_list>

namespace beman::utf_view::detail {

template <typename T, std::size_t N>
class fake_inplace_vector {
public:
  constexpr fake_inplace_vector() = default;
  constexpr fake_inplace_vector(std::initializer_list<T> init)
  {
    std::ranges::copy(init, storage_);
  }

  constexpr std::size_t size() const {
    return size_;
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
  T* begin() {
    return &storage_[0];
  }
  T* end() {
    return &storage_[size_];
  }

private:
  T storage_[N];
  size_t size_{};
};

} // namespace beman::utf_view::detail

#endif // BEMAN_UTF_VIEW_FAKE_INPLACE_VECTOR_HPP
