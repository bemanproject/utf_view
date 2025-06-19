// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef BEMAN_UTF_VIEW_NULL_TERM_HPP
#define BEMAN_UTF_VIEW_NULL_TERM_HPP

#include <concepts>
#include <ranges>
#include <utility>

namespace beman::utf_view {

/* PAPER: namespace std { */

/* PAPER */

template <class I>
concept exposition_only_default_initializable_and_equality_comparable_iter_value =
    std::default_initializable<std::iter_value_t<I>> &&
    std::equality_comparable_with<std::iter_reference_t<I>,
                                  std::iter_value_t<I>>; // @*exposition only*@

struct null_sentinel_t {
  template <std::input_iterator I>
  requires exposition_only_default_initializable_and_equality_comparable_iter_value<I>
  friend constexpr bool operator==(I const& it, null_sentinel_t) {
    return *it == std::iter_value_t<I>();
  }
};

inline constexpr null_sentinel_t null_sentinel;

/* !PAPER */

namespace detail {

  struct null_term_impl {
    template <typename I>
    constexpr auto operator()(I&& it) const {
      return std::ranges::subrange(std::forward<I>(it), null_sentinel);
    }
  };

} // namespace detail

inline constexpr detail::null_term_impl null_term;

/* PAPER: inline constexpr @*unspecified*@ null_term; */
/* PAPER: */

} // namespace beman::utf_view

/* PAPER: } */

#endif // BEMAN_UTF_VIEW_NULL_TERM_HPP
