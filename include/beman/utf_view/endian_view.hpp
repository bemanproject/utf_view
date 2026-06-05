// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef BEMAN_UTF_VIEW_ENDIAN_VIEW_HPP
#define BEMAN_UTF_VIEW_ENDIAN_VIEW_HPP

#include <beman/utf_view/config.hpp>

#if BEMAN_UTF_VIEW_USE_MODULES() && \
    !defined(BEMAN_UTF_VIEW_INCLUDED_FROM_INTERFACE_UNIT)

import beman.utf_view;

#else

#include <beman/utf_view/detail/concepts.hpp>
#if !BEMAN_UTF_VIEW_USE_MODULES()
#include <beman/transform_view/transform_view.hpp>
#include <bit>
#include <ranges>
#include <type_traits>
#endif

namespace beman::utf_view {

/* PAPER: namespace std::ranges::views { */

namespace detail {

/* PAPER */

struct exposition_only_byteswap {
  constexpr auto operator()(auto x) const noexcept {
    return std::byteswap(x);
  }
};

/* !PAPER */

} // namespace detail

namespace detail {

  template <std::endian endianness>
  struct from_to_endian_impl
      : std::ranges::range_adaptor_closure<from_to_endian_impl<endianness>> {
    template <std::ranges::range R>
      requires std::integral<std::ranges::range_value_t<R>>
    constexpr auto operator()(R&& r) const {
      using T = std::remove_cvref_t<R>;
      if constexpr (detail::is_empty_view<T>) {
        return std::ranges::empty_view<std::ranges::range_value_t<T>>{};
      } else {
        if constexpr (std::endian::native == endianness) {
          return std::views::all(std::forward<R>(r));
        } else {
          return beman::transform_view::transform_view(
              std::forward<R>(r), exposition_only_byteswap{});
        }
      }
    }
  };

} // namespace detail

inline constexpr detail::from_to_endian_impl<std::endian::little> from_little_endian;

inline constexpr detail::from_to_endian_impl<std::endian::big> from_big_endian;

inline constexpr detail::from_to_endian_impl<std::endian::little> to_little_endian;

inline constexpr detail::from_to_endian_impl<std::endian::big> to_big_endian;

/* PAPER:   inline constexpr @*unspecified*@ from_little_endian = @*unspecified*@;  */
/* PAPER:   inline constexpr @*unspecified*@ from_big_endian = @*unspecified*@;     */
/* PAPER:   inline constexpr @*unspecified*@ to_little_endian = @*unspecified*@;    */
/* PAPER:   inline constexpr @*unspecified*@ to_big_endian = @*unspecified*@;       */

/* PAPER: } */

} // namespace beman::utf_view

#endif // BEMAN_UTF_VIEW_USE_MODULES() &&
       // !defined(BEMAN_UTF_VIEW_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_UTF_VIEW_ENDIAN_VIEW_HPP
