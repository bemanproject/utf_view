// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef BEMAN_UTF_VIEW_DETAIL_CONCEPTS_HPP
#define BEMAN_UTF_VIEW_DETAIL_CONCEPTS_HPP

#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>

namespace beman::utf_view {

/* PAPER: namespace std::ranges { */

template <class T>
using exposition_only_with_reference = T&; // exposition only
template <class T>
concept exposition_only_can_reference // exposition only
    = requires { typename exposition_only_with_reference<T>; };

template <bool Const, class T>
using exposition_only_maybe_const =
    std::conditional_t<Const, const T, T>; // exposition only

/* PAPER */

template <class T>
constexpr bool exposition_only_is_empty_view = false;
template <class T>
constexpr bool exposition_only_is_empty_view<std::ranges::empty_view<T>> = true;

template <class T>
concept exposition_only_code_unit = std::same_as<std::remove_cv_t<T>, char8_t> ||
    std::same_as<std::remove_cv_t<T>, char16_t> ||
    std::same_as<std::remove_cv_t<T>, char32_t>;

/* !PAPER */

} // namespace beman::utf_view

/* PAPER: } */

#endif // BEMAN_UTF_VIEW_DETAIL_CONCEPTS_HPP
