// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <beman/utf_view/code_unit_view.hpp>
#include <beman/utf_view/detail/constexpr_unless_msvc.hpp>
#include <framework.hpp>
#include <test_iterators.hpp>
#include <cstdint>
#include <ranges>
#include <string_view>

namespace beman::utf_view::tests {

static_assert(
  std::input_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<test_input_iterator<char>, std::default_sentinel_t>>()
          | as_char8_t)>>);
static_assert(
  std::input_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<
                  test_comparable_input_iterator<char>, std::default_sentinel_t>>()
          | as_char8_t)>>);
static_assert(
  std::input_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<
                  test_copyable_input_iterator<char>, std::default_sentinel_t>>()
          | as_char8_t)>>);
static_assert(
  !std::forward_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<
                  test_copyable_input_iterator<char>, std::default_sentinel_t>>()
          | as_char8_t)>>);

static_assert(
  std::forward_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<
                  test_forward_iterator<char>, std::default_sentinel_t>>()
          | as_char8_t)>>);
static_assert(
  std::forward_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<
                  test_forward_iterator<char>, test_forward_iterator<char>>>()
          | as_char8_t)>>);

static_assert(
  std::bidirectional_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<test_bidi_iterator<char>, std::default_sentinel_t>>()
          | as_char8_t)>>);
static_assert(
  std::bidirectional_iterator<
    std::ranges::sentinel_t<
        decltype(
          std::declval<
              std::ranges::subrange<test_bidi_iterator<char>, test_bidi_iterator<char>>>()
          | as_char8_t)>>);

static_assert(
  std::random_access_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<
                  test_random_access_iterator<char>, std::default_sentinel_t>>()
          | as_char8_t)>>);
static_assert(
  std::random_access_iterator<
    std::ranges::sentinel_t<
        decltype(
          std::declval<
              std::ranges::subrange<
                  test_random_access_iterator<char>, test_random_access_iterator<char>>>()
          | as_char8_t)>>);

constexpr bool smoke_test() {
  std::string_view foo{"foo"};
  auto bar{foo | as_char8_t};
  static_assert(std::ranges::borrowed_range<decltype(foo)>);
  auto baz{bar | std::ranges::to<std::u8string>()};
  if (baz != std::u8string_view{u8"foo"}) {
    return false;
  }
  return true;
}

constexpr bool special_case_test() {
  std::ranges::empty_view<std::uint8_t> empty_int_view{};
  auto empty_char8_view{empty_int_view | as_char8_t};
  static_assert(
      std::is_same_v<decltype(empty_char8_view), std::ranges::empty_view<char8_t>>);
  return true;
}

CONSTEXPR_UNLESS_MSVC bool code_unit_view_test() {
  if (!smoke_test()) {
    return false;
  }
  if (!special_case_test()) {
    return false;
  }
  return true;
}

#ifndef _MSC_VER
static_assert(code_unit_view_test());
#endif

static auto const init{[] {
  framework::tests().insert({"code_unit_view_test", &code_unit_view_test});
  struct {
  } result{};
  return result;
}()};

} // namespace beman::utf_view::tests
