// SPDX-License-Identifier: GPL-3.0-only

#ifndef UTFVIEW_DETAIL_CONCEPTS_HPP
#define UTFVIEW_DETAIL_CONCEPTS_HPP

#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>

namespace utfview {

/* PAPER: namespace std::uc { */

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
concept exposition_only_code_unit_to = std::same_as<std::remove_cv_t<T>, char8_t> ||
    std::same_as<std::remove_cv_t<T>, char16_t> ||
    std::same_as<std::remove_cv_t<T>, char32_t>;

template <class T>
concept exposition_only_code_unit_from = std::same_as<std::remove_cv_t<T>, char> ||
    std::same_as<std::remove_cv_t<T>, wchar_t> || exposition_only_code_unit_to<T>;

template <class T>
concept exposition_only_utf_range = std::ranges::input_range<T> &&
    exposition_only_code_unit_from<std::ranges::range_value_t<T>>;

/* !PAPER */

} // namespace utfview

/* PAPER: } */

#endif // UTFVIEW_DETAIL_CONCEPTS_HPP
