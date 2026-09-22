// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan 2023 - 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <beman/utf_view/config.hpp>
#include <beman/utf_view/code_unit_view.hpp>
#include <beman/utf_view/detail/constexpr_unless_msvc.hpp>
#include <framework.hpp>
#include <test_iterators.hpp>
#if BEMAN_UTF_VIEW_USE_MODULES()
import std;
#else
#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#endif

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

// Conversion rule: a source type is accepted if it is the target type, or if it
// is std::byte or a non-bool integral type of the same width as the target.
template <class R, auto const& Adaptor>
constexpr bool adaptable = requires { std::declval<R>() | Adaptor; };

enum unscoped_byte_enum : unsigned char { unscoped_byte_enum_value };
enum class scoped_byte_enum : unsigned char { value };
struct convertible_to_char8_t {
  constexpr operator char8_t() const { return u8'x'; }
};

// as_char8_t
static_assert(adaptable<std::string_view, as_char8_t>);
static_assert(adaptable<std::u8string_view, as_char8_t>);
static_assert(adaptable<std::span<signed char const>, as_char8_t>);
static_assert(adaptable<std::span<unsigned char const>, as_char8_t>);
static_assert(adaptable<std::span<std::byte const>, as_char8_t>);
static_assert(adaptable<std::span<std::uint8_t const>, as_char8_t>);
static_assert(adaptable<std::span<std::int8_t const>, as_char8_t>);
static_assert(!adaptable<std::u16string_view, as_char8_t>);
static_assert(!adaptable<std::u32string_view, as_char8_t>);
static_assert(!adaptable<std::wstring_view, as_char8_t>);
static_assert(!adaptable<std::span<short const>, as_char8_t>);
static_assert(!adaptable<std::span<int const>, as_char8_t>);
static_assert(!adaptable<std::span<bool const>, as_char8_t>);
static_assert(!adaptable<std::vector<bool>&, as_char8_t>);
static_assert(!adaptable<std::span<float const>, as_char8_t>);
static_assert(!adaptable<std::span<double const>, as_char8_t>);
static_assert(!adaptable<std::span<unscoped_byte_enum const>, as_char8_t>);
static_assert(!adaptable<std::span<scoped_byte_enum const>, as_char8_t>);
static_assert(!adaptable<std::span<convertible_to_char8_t const>, as_char8_t>);
static_assert(!adaptable<char const (&)[4], as_char8_t>);

// as_char
static_assert(adaptable<std::string_view, as_char>);
static_assert(adaptable<std::u8string_view, as_char>);
static_assert(adaptable<std::span<unsigned char const>, as_char>);
static_assert(adaptable<std::span<std::byte const>, as_char>);
static_assert(!adaptable<std::u16string_view, as_char>);
static_assert(!adaptable<std::u32string_view, as_char>);

// as_char16_t
static_assert(adaptable<std::u16string_view, as_char16_t>);
static_assert(adaptable<std::span<short const>, as_char16_t>);
static_assert(adaptable<std::span<unsigned short const>, as_char16_t>);
static_assert(adaptable<std::span<std::uint16_t const>, as_char16_t>);
static_assert(adaptable<std::wstring_view, as_char16_t> ==
              (sizeof(wchar_t) == sizeof(char16_t)));
static_assert(!adaptable<std::u8string_view, as_char16_t>);
static_assert(!adaptable<std::u32string_view, as_char16_t>);
static_assert(!adaptable<std::string_view, as_char16_t>);
static_assert(!adaptable<std::span<std::byte const>, as_char16_t>);

// as_char32_t
static_assert(adaptable<std::u32string_view, as_char32_t>);
static_assert(adaptable<std::span<std::uint32_t const>, as_char32_t>);
static_assert(adaptable<std::span<std::int32_t const>, as_char32_t>);
static_assert(adaptable<std::span<int const>, as_char32_t> ==
              (sizeof(int) == sizeof(char32_t)));
static_assert(adaptable<std::span<unsigned const>, as_char32_t> ==
              (sizeof(unsigned) == sizeof(char32_t)));
static_assert(adaptable<std::span<long const>, as_char32_t> ==
              (sizeof(long) == sizeof(char32_t)));
static_assert(adaptable<std::wstring_view, as_char32_t> ==
              (sizeof(wchar_t) == sizeof(char32_t)));
static_assert(!adaptable<std::u8string_view, as_char32_t>);
static_assert(!adaptable<std::u16string_view, as_char32_t>);
static_assert(!adaptable<std::span<long long const>, as_char32_t>);
static_assert(!adaptable<std::span<double const>, as_char32_t>);

// as_wchar_t
static_assert(adaptable<std::wstring_view, as_wchar_t>);
static_assert(adaptable<std::u16string_view, as_wchar_t> ==
              (sizeof(wchar_t) == sizeof(char16_t)));
static_assert(adaptable<std::u32string_view, as_wchar_t> ==
              (sizeof(wchar_t) == sizeof(char32_t)));
static_assert(adaptable<std::span<unsigned short const>, as_wchar_t> ==
              (sizeof(wchar_t) == sizeof(unsigned short)));
static_assert(!adaptable<std::u8string_view, as_wchar_t>);
static_assert(!adaptable<std::string_view, as_wchar_t>);

constexpr bool smoke_test() {
  std::string_view foo{"foo"};
  auto bar{foo | as_char8_t};
  static_assert(std::ranges::borrowed_range<decltype(bar)>);
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

constexpr bool same_type_test() {
  std::u8string_view foo{u8"foo"};
  auto bar{foo | as_char8_t};
  if ((bar | std::ranges::to<std::u8string>()) != foo) {
    return false;
  }
  return true;
}

constexpr bool byte_test() {
  std::array<std::byte, 3> const euro{std::byte{0xE2}, std::byte{0x82},
                                      std::byte{0xAC}};
  auto const as_utf8{euro | as_char8_t | std::ranges::to<std::u8string>()};
  if (as_utf8 != std::u8string_view{u8"\u20AC"}) {
    return false;
  }
  return true;
}

// The cast is as if by static_cast, so negative values in a signed source
// preserve their bit pattern.
constexpr bool signed_source_test() {
  std::array<signed char, 3> const euro{static_cast<signed char>(-30),
                                        static_cast<signed char>(-126),
                                        static_cast<signed char>(-84)};
  auto const as_utf8{euro | as_char8_t | std::ranges::to<std::u8string>()};
  if (as_utf8 != std::u8string_view{u8"\u20AC"}) {
    return false;
  }
  return true;
}

constexpr bool int_source_test() {
  if constexpr (sizeof(int) == sizeof(char32_t)) {
    std::array<int, 2> const code_points{0x1F574, 0xFFFD};
    auto const as_utf32{code_points | as_char32_t |
                        std::ranges::to<std::u32string>()};
    if (as_utf32 != std::u32string_view{U"\U0001F574\uFFFD"}) {
      return false;
    }
  }
  return true;
}

constexpr bool value_category_test() {
  std::string foo{"foo"};
  (void)(foo | as_char8_t);
  if (foo.empty()) {
    return false;
  }
  return true;
}

CONSTEXPR_UNLESS_MSVC bool code_unit_view_test() {
  if (!smoke_test()) {
    return false;
  }
  if (!special_case_test()) {
    return false;
  }
  if (!same_type_test()) {
    return false;
  }
  if (!byte_test()) {
    return false;
  }
  if (!signed_source_test()) {
    return false;
  }
  if (!int_source_test()) {
    return false;
  }
  if (!value_category_test()) {
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
