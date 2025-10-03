// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef BEMAN_UTF_VIEW_CODE_UNIT_VIEW_HPP
#define BEMAN_UTF_VIEW_CODE_UNIT_VIEW_HPP

#include <beman/utf_view/detail/concepts.hpp>
#include <beman/transform_view/transform_view.hpp>
#include <ranges>
#include <type_traits>

namespace beman::utf_view {

/* PAPER: namespace std::ranges::views { */

namespace detail {

/* PAPER */

template <class T>
struct exposition_only_implicit_cast_to {
  constexpr T operator()(auto x) const noexcept {
    return x;
  }
};

/* !PAPER */

} // namespace detail

namespace detail {

  template <exposition_only_code_unit Char>
  struct as_code_unit_impl
      : std::ranges::range_adaptor_closure<as_code_unit_impl<Char>> {
    template <std::ranges::range R>
      requires std::convertible_to<std::ranges::range_reference_t<R>, Char>
    constexpr auto operator()(R&& r) const {
      using T = std::remove_cvref_t<R>;
      if constexpr (exposition_only_is_empty_view<T>) {
        return std::ranges::empty_view<Char>{};
      } else if constexpr (std::is_bounded_array_v<T>) {
        constexpr auto n = std::extent_v<T>;
        auto first{std::ranges::begin(r)};
        auto last{std::ranges::end(r)};
        if (n && !r[n - 1]) {
          --last;
        }
        std::ranges::subrange subrange(first, last);
        return beman::transform_view::transform_view(
            std::move(subrange), exposition_only_implicit_cast_to<Char>{});
      } else {
        return beman::transform_view::transform_view(
            std::move(r), exposition_only_implicit_cast_to<Char>{});
      }
    }
  };

} // namespace detail

inline constexpr detail::as_code_unit_impl<char8_t> as_char8_t;

inline constexpr detail::as_code_unit_impl<char16_t> as_char16_t;

inline constexpr detail::as_code_unit_impl<char32_t> as_char32_t;

/* PAPER:   inline constexpr @*unspecified*@ as_char8_t;  */
/* PAPER:                                                 */
/* PAPER:   inline constexpr @*unspecified*@ as_char16_t; */
/* PAPER:                                                 */
/* PAPER:   inline constexpr @*unspecified*@ as_char32_t; */

/* PAPER: } */

} // namespace beman::utf_view

#endif // BEMAN_UTF_VIEW_CODE_UNIT_VIEW_HPP
