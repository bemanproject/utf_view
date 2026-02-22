// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <beman/utf_view/endian_view.hpp>
#include <beman/utf_view/code_unit_view.hpp>
#include <beman/utf_view/detail/constexpr_unless_msvc.hpp>
#include <beman/utf_view/to_utf_view.hpp>
#include <framework.hpp>
#include <test_iterators.hpp>
#include <array>
#include <bit>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <string_view>
#include <vector>

namespace beman::utf_view::tests {

static_assert(
  std::input_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<test_input_iterator<int>, std::default_sentinel_t>>()
          | from_big_endian)>>);
static_assert(
  std::input_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<
                  test_comparable_input_iterator<int>, std::default_sentinel_t>>()
          | from_big_endian)>>);
static_assert(
  std::input_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<
                  test_copyable_input_iterator<int>, std::default_sentinel_t>>()
          | from_big_endian)>>);
static_assert(
  !std::forward_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<
                  test_copyable_input_iterator<int>, std::default_sentinel_t>>()
          | from_big_endian)>>);

static_assert(
  std::forward_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<
                  test_forward_iterator<int>, std::default_sentinel_t>>()
          | from_big_endian)>>);
static_assert(
  std::forward_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<
                  test_forward_iterator<int>, test_forward_iterator<int>>>()
          | from_big_endian)>>);

static_assert(
  std::bidirectional_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<test_bidi_iterator<int>, std::default_sentinel_t>>()
          | from_big_endian)>>);
static_assert(
  std::bidirectional_iterator<
    std::ranges::sentinel_t<
        decltype(
          std::declval<
              std::ranges::subrange<test_bidi_iterator<int>, test_bidi_iterator<int>>>()
          | from_big_endian)>>);

static_assert(
  std::random_access_iterator<
    std::ranges::iterator_t<
        decltype(
          std::declval<
              std::ranges::subrange<
                  test_random_access_iterator<int>, std::default_sentinel_t>>()
          | from_big_endian)>>);
static_assert(
  std::random_access_iterator<
    std::ranges::sentinel_t<
        decltype(
          std::declval<
              std::ranges::subrange<
                  test_random_access_iterator<int>, test_random_access_iterator<int>>>()
          | from_big_endian)>>);

constexpr bool smoke_test() {
  std::vector<int> foo{0x12345678};
  auto bar{foo | from_big_endian};
  static_assert(std::ranges::borrowed_range<decltype(bar)>);
  auto baz{bar | std::ranges::to<std::vector<int>>()};
  if (baz != std::vector<int>{0x78563412}) {
    return false;
  }
  return true;
}

constexpr bool special_case_test() {
  std::ranges::empty_view<int> empty_int_view{};
  auto empty_int_view2{empty_int_view | from_big_endian};
  static_assert(
      std::is_same_v<decltype(empty_int_view2), std::ranges::empty_view<int>>);
  return true;
}

constexpr bool value_category_test() {
  std::vector<int> foo{0x12345678};
  (void)(foo | from_big_endian);
  if (foo.empty()) {
    return false;
  }
  return true;
}

constexpr std::vector<std::uint32_t> utf16be_to_utf32be_before(
    const std::vector<std::uint16_t>& utf16be_data) {
  return utf16be_data
    | std::views::transform(
        [](const uint16_t x) {
          if constexpr (std::endian::native == std::endian::little) {
            return std::byteswap(x);
          } else {
            return x;
          }
        })
    | as_char16_t
    | to_utf32
    | std::views::transform(
        [](const char32_t c) {
          const auto x = static_cast<uint32_t>(c);
          if constexpr (std::endian::native == std::endian::little) {
            return std::byteswap(x);
          } else {
            return x;
          }
        })
    | std::ranges::to<std::vector>();
}

constexpr std::vector<std::uint32_t> utf16be_to_utf32be_after(
    const std::vector<uint16_t>& utf16be_data) {
  return utf16be_data
    | from_big_endian
    | as_char16_t
    | to_utf32
    | std::views::transform(
        [](const char32_t c) {
          return static_cast<std::uint32_t>(c);
        })
    | to_big_endian
    | std::ranges::to<std::vector>();
}

#if __cpp_lib_ranges_concat >= 202403L
constexpr std::vector<std::byte> synthesize_tls_client_hello_before(
    const std::vector<std::uint16_t>& cipher_suites) {
  std::vector<std::byte> result;
  // ...
  // TLS ClientHello CipherSuite list is a
  // length-prefixed sequence of 16-bit
  // big-endian values
  std::ranges::copy(
    std::views::concat(
        std::views::single(
          static_cast<std::uint16_t>(cipher_suites.size())),
      cipher_suites)
    | std::views::transform(
        [](const std::uint16_t x) {
          if constexpr (std::endian::native == std::endian::little) {
            return std::byteswap(x);
          } else {
            return x;
          }
        })
    | std::views::transform(
        [](const std::uint16_t x) {
          return std::bit_cast<std::array<std::byte, 2>>(x);
        })
    | std::views::join,
    std::back_insert_iterator{result});
  // ...
  return result;
}

constexpr std::vector<std::byte> synthesize_tls_client_hello_after(
    const std::vector<std::uint16_t>& cipher_suites) {
  std::vector<std::byte> result;
  // ...
  // TLS ClientHello CipherSuite list is a
  // length-prefixed sequence of 16-bit
  // big-endian values
  std::ranges::copy(
    std::views::concat(
      std::views::single(
        static_cast<std::uint16_t>(cipher_suites.size())),
      cipher_suites)
    | to_big_endian
    | std::views::transform(
        [](const std::uint16_t x) {
          return std::bit_cast<std::array<std::byte, 2>>(x);
        })
    | std::views::join,
    std::back_insert_iterator{result});
  // ...
  return result;
}
#endif

constexpr bool before_after_tests() {
  if (!std::ranges::equal(
        std::vector<std::uint32_t>{0x42F60100},
        utf16be_to_utf32be_before(std::vector<std::uint16_t>{0x3DD8, 0x42DE}))) {
    return false;
  }
  if (!std::ranges::equal(
        std::vector<std::uint32_t>{0x42F60100},
        utf16be_to_utf32be_after(std::vector<std::uint16_t>{0x3DD8, 0x42DE}))) {
    return false;
  }
#if __cpp_lib_ranges_concat >= 202403L
  if (!std::ranges::equal(
        std::vector<std::byte>{std::byte{0x00}, std::byte{0x02}, std::byte{0x12},
                               std::byte{0x34}, std::byte{0x56}, std::byte{0x78}},
        synthesize_tls_client_hello_before({0x1234, 0x5678}))) {
    return false;
  }
  if (!std::ranges::equal(
        std::vector<std::byte>{std::byte{0x00}, std::byte{0x02}, std::byte{0x12},
                               std::byte{0x34}, std::byte{0x56}, std::byte{0x78}},
        synthesize_tls_client_hello_after({0x1234, 0x5678}))) {
    return false;
  }
#endif
  return true;
}

CONSTEXPR_UNLESS_MSVC bool endian_view_test() {
  if (!smoke_test()) {
    return false;
  }
  if (!special_case_test()) {
    return false;
  }
  if (!value_category_test()) {
    return false;
  }
  if (!before_after_tests()) {
    return false;
  }
  return true;
}

#ifndef _MSC_VER
static_assert(endian_view_test());
#endif

static auto const init{[] {
  framework::tests().insert({"endian_view_test", &endian_view_test});
  struct {
  } result{};
  return result;
}()};

} // namespace beman::utf_view::tests
