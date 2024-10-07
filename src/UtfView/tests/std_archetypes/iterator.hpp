// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef UTFVIEW_TESTS_STD_ARCHETYPES_ITERATOR_HPP
#define UTFVIEW_TESTS_STD_ARCHETYPES_ITERATOR_HPP

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <tests/std_archetypes/exposition_only.hpp>

namespace utfview::tests::std_archetypes {

#ifdef _MSC_VER
// https://developercommunity.visualstudio.com/t/MSVC-fails-to-resolve-operator-overloa/10560905
using boolean_testable_archetype = bool;
#else
using boolean_testable_archetype = boolean_testable_archetype1;
#endif

namespace detail {

  template <typename State>
  struct reference_type_archetype;

  template <typename State>
  struct value_type_archetype {
    constexpr value_type_archetype()
        : x{} { }
    constexpr value_type_archetype(reference_type_archetype<State> const& ref);
    value_type_archetype(value_type_archetype const&) = default;
    value_type_archetype& operator=(value_type_archetype const&) = default;
    value_type_archetype(value_type_archetype&&) = default;
    value_type_archetype& operator=(value_type_archetype&&) = default;
    constexpr boolean_testable_archetype operator==(
        value_type_archetype const& other) const {
      return boolean_testable_archetype{x == other.x};
    }
    constexpr boolean_testable_archetype operator!=(
        value_type_archetype const& other) const {
      return boolean_testable_archetype{x != other.x};
    }
    State x;
  };

  template <typename State>
  struct reference_type_archetype {
    constexpr boolean_testable_archetype operator==(
        value_type_archetype<State> const& value) const {
      return boolean_testable_archetype{x == value.x};
    }
    constexpr boolean_testable_archetype operator!=(
        value_type_archetype<State> const& value) const {
      return boolean_testable_archetype{x != value.x};
    }
    State const& x;
  };

  template <typename State>
  constexpr value_type_archetype<State>::value_type_archetype(
      reference_type_archetype<State> const& ref)
      : x{ref.x} { }

} // namespace detail

template <std::movable State, typename ValueType = detail::value_type_archetype<State>,
          typename ReferenceType = detail::reference_type_archetype<State>>
struct basic_input_iterator_archetype {
  using value_type = ValueType;
  using reference_type = ReferenceType;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::input_iterator_tag;
  constexpr explicit basic_input_iterator_archetype(State x_in)
      : x{std::move(x_in)} { }
  basic_input_iterator_archetype(basic_input_iterator_archetype const&) = delete;
  basic_input_iterator_archetype& operator=(basic_input_iterator_archetype const&) =
      delete;
  basic_input_iterator_archetype(basic_input_iterator_archetype&&) = default;
  basic_input_iterator_archetype& operator=(basic_input_iterator_archetype&&) = default;
  constexpr reference_type operator*() const {
    return reference_type{x};
  }
  constexpr basic_input_iterator_archetype& operator++() {
    ++x;
    return *this;
  }
  constexpr void operator++(int) {
    ++x;
  }
  State x;
};

using input_iterator_archetype = basic_input_iterator_archetype<std::uint8_t>;

template <std::copyable State, typename ValueType = detail::value_type_archetype<State>,
          typename ReferenceType = detail::reference_type_archetype<State>>
struct basic_forward_iterator_archetype {
  using value_type = ValueType;
  using reference_type = ReferenceType;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::forward_iterator_tag;
  constexpr basic_forward_iterator_archetype()
      : x{} { }
  constexpr explicit basic_forward_iterator_archetype(State x_in)
      : x{std::move(x_in)} { }
  basic_forward_iterator_archetype(basic_forward_iterator_archetype const&) = default;
  basic_forward_iterator_archetype& operator=(basic_forward_iterator_archetype const&) =
      default;
  basic_forward_iterator_archetype(basic_forward_iterator_archetype&&) = default;
  basic_forward_iterator_archetype& operator=(basic_forward_iterator_archetype&&) =
      default;

  constexpr reference_type operator*() const {
    return reference_type{x};
  }
  constexpr basic_forward_iterator_archetype& operator++() {
    ++x;
    return *this;
  }
  constexpr basic_forward_iterator_archetype operator++(int) {
    basic_forward_iterator_archetype result{*this};
    ++x;
    return result;
  }

  friend boolean_testable_archetype operator==(
      basic_forward_iterator_archetype const& lhs,
      basic_forward_iterator_archetype const& rhs) {
    return boolean_testable_archetype{lhs.x == rhs.x};
  }
  friend boolean_testable_archetype operator!=(
      basic_forward_iterator_archetype const& lhs,
      basic_forward_iterator_archetype const& rhs) {
    return boolean_testable_archetype{lhs.x != rhs.x};
  }

  State x;
};

using forward_iterator_archetype = basic_forward_iterator_archetype<std::uint8_t>;

template <std::copyable State, typename ValueType = detail::value_type_archetype<State>,
          typename ReferenceType = detail::reference_type_archetype<State>>
struct basic_bidirectional_iterator_archetype {
  using value_type = ValueType;
  using reference_type = ReferenceType;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::bidirectional_iterator_tag;
  constexpr basic_bidirectional_iterator_archetype()
      : x{} { }
  constexpr explicit basic_bidirectional_iterator_archetype(State x_in)
      : x{std::move(x_in)} { }
  basic_bidirectional_iterator_archetype(basic_bidirectional_iterator_archetype const&) =
      default;
  basic_bidirectional_iterator_archetype& operator=(
      basic_bidirectional_iterator_archetype const&) = default;
  basic_bidirectional_iterator_archetype(basic_bidirectional_iterator_archetype&&) =
      default;
  basic_bidirectional_iterator_archetype& operator=(
      basic_bidirectional_iterator_archetype&&) = default;

  constexpr reference_type operator*() const {
    return reference_type{x};
  }
  constexpr basic_bidirectional_iterator_archetype& operator++() {
    ++x;
    return *this;
  }
  constexpr basic_bidirectional_iterator_archetype operator++(int) {
    basic_bidirectional_iterator_archetype result{*this};
    ++x;
    return result;
  }
  constexpr basic_bidirectional_iterator_archetype& operator--() {
    --x;
    return *this;
  }
  constexpr basic_bidirectional_iterator_archetype operator--(int) {
    basic_bidirectional_iterator_archetype result{*this};
    --x;
    return result;
  }

  friend boolean_testable_archetype operator==(
      basic_bidirectional_iterator_archetype const& lhs,
      basic_bidirectional_iterator_archetype const& rhs) {
    return boolean_testable_archetype{lhs.x == rhs.x};
  }
  friend boolean_testable_archetype operator!=(
      basic_bidirectional_iterator_archetype const& lhs,
      basic_bidirectional_iterator_archetype const& rhs) {
    return boolean_testable_archetype{lhs.x != rhs.x};
  }

  State x;
};

using bidirectional_iterator_archetype =
    basic_bidirectional_iterator_archetype<std::uint8_t>;

} // namespace utfview::tests::std_archetypes

#endif // UTFVIEW_TESTS_STD_ARCHETYPES_ITERATOR_HPP
