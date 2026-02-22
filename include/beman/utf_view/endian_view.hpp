// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef BEMAN_UTF_VIEW_ENDIAN_VIEW_HPP
#define BEMAN_UTF_VIEW_ENDIAN_VIEW_HPP

#include <beman/utf_view/detail/concepts.hpp>
#include <beman/transform_view/transform_view.hpp>
#include <bit>
#include <ranges>
#include <type_traits>

namespace beman::utf_view {

/* PAPER: namespace std::ranges::views { */

namespace detail {

/* PAPER */

struct exposition_only_byteswap_if_native_is_big_endian {
  constexpr auto operator()(auto x) const noexcept {
    if constexpr (std::endian::native == std::endian::big) {
      return std::byteswap(x);
    } else {
      return x;
    }
  }
};

struct exposition_only_byteswap_if_native_is_little_endian { // @*exposition only*@/
  constexpr auto operator()(auto x) const noexcept {
    if constexpr (std::endian::native == std::endian::little) {
      return std::byteswap(x);
    } else {
      return x;
    }
  }
};

/* !PAPER */

} // namespace detail

namespace detail {

  template <bool byteswap_if_native_is_big>
  struct from_to_endian_impl
      : std::ranges::range_adaptor_closure<from_to_endian_impl<byteswap_if_native_is_big>> {
    template <std::ranges::range R>
      requires std::integral<std::ranges::range_value_t<R>>
    constexpr auto operator()(R&& r) const {
      using T = std::remove_cvref_t<R>;
      if constexpr (detail::is_empty_view<T>) {
        return std::ranges::empty_view<std::ranges::range_value_t<T>>{};
      } else {
        if constexpr (byteswap_if_native_is_big) {
          return beman::transform_view::transform_view(
              std::forward<R>(r), exposition_only_byteswap_if_native_is_big_endian{});
        } else {
          return beman::transform_view::transform_view(
              std::forward<R>(r), exposition_only_byteswap_if_native_is_little_endian{});
        }
      }
    }
  };

} // namespace detail

inline constexpr detail::from_to_endian_impl<true> from_little_endian;

inline constexpr detail::from_to_endian_impl<false> from_big_endian;

inline constexpr detail::from_to_endian_impl<true> to_little_endian;

inline constexpr detail::from_to_endian_impl<false> to_big_endian;

/* PAPER:   inline constexpr @*unspecified*@ from_little_endian;  */
/* PAPER:                                                         */
/* PAPER:   inline constexpr @*unspecified*@ from_big_endian;     */
/* PAPER:                                                         */
/* PAPER:   inline constexpr @*unspecified*@ to_little_endian;    */
/* PAPER:                                                         */
/* PAPER:   inline constexpr @*unspecified*@ to_big_endian;       */

/* PAPER: } */

} // namespace beman::utf_view

#endif // BEMAN_UTF_VIEW_ENDIAN_VIEW_HPP
