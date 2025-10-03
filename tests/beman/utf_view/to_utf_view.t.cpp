// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <beman/utf_view/detail/constexpr_unless_msvc.hpp>
#include <beman/utf_view/null_term.hpp>
#include <beman/utf_view/to_utf_view.hpp>
#include <framework.hpp>
#include <test_iterators.hpp>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <ios>
#include <iterator>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>

namespace beman::utf_view::tests {

static_assert(
  std::input_iterator<
    std::ranges::iterator_t<
      to_utf8_view<
        std::ranges::subrange<test_input_iterator<char8_t>, std::default_sentinel_t>>>>);

static_assert(
  std::input_iterator<
    std::ranges::iterator_t<
      to_utf8_view<
        std::ranges::subrange<test_comparable_input_iterator<char8_t>,
                              std::default_sentinel_t>>>>);

static_assert(
  std::input_iterator<
    std::ranges::iterator_t<
      to_utf8_view<
        std::ranges::subrange<test_copyable_input_iterator<char8_t>,
                              std::default_sentinel_t>>>>);
static_assert(
  !std::forward_iterator<
    std::ranges::iterator_t<
      to_utf8_view<
        std::ranges::subrange<test_copyable_input_iterator<char8_t>,
                              std::default_sentinel_t>>>>);

static_assert(
  std::forward_iterator<
    std::ranges::iterator_t<
      to_utf8_view<
        std::ranges::subrange<test_forward_iterator<char8_t>,
                              std::default_sentinel_t>>>>);
static_assert(
  std::forward_iterator<
    std::ranges::sentinel_t<
      to_utf8_view<
        std::ranges::subrange<test_forward_iterator<char8_t>,
                              test_forward_iterator<char8_t>>>>>);
static_assert(
  std::bidirectional_iterator<
    std::ranges::iterator_t<
      to_utf8_view<
        std::ranges::subrange<test_bidi_iterator<char8_t>,
                              std::default_sentinel_t>>>>);
static_assert(
  std::bidirectional_iterator<
    std::ranges::sentinel_t<
      to_utf8_view<
        std::ranges::subrange<test_bidi_iterator<char8_t>,
                              test_bidi_iterator<char8_t>>>>>);

template <exposition_only_code_unit_to CharT>
using test_case_code_unit_result = std::expected<CharT, utf_transcoding_error>;

template <exposition_only_code_unit_from CharTFrom, exposition_only_code_unit_to CharTTo>
struct test_case {
  std::initializer_list<CharTFrom> input;
  std::initializer_list<test_case_code_unit_result<CharTTo>> output;
};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char32_t> table3_8{
    .input{static_cast<char8_t>('\xc0'), static_cast<char8_t>('\xaf'),
           static_cast<char8_t>('\xe0'), static_cast<char8_t>('\x80'),
           static_cast<char8_t>('\xbf'), static_cast<char8_t>('\xf0'),
           static_cast<char8_t>('\x81'), static_cast<char8_t>('\x82'),
           static_cast<char8_t>('A')},
    .output{{std::unexpected{utf_transcoding_error::invalid_utf8_leading_byte}},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {std::unexpected{utf_transcoding_error::overlong}},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {std::unexpected{utf_transcoding_error::overlong}},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {U'A'}}};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char32_t> table3_9{
    .input{static_cast<char8_t>('\xed'), static_cast<char8_t>('\xa0'),
           static_cast<char8_t>('\x80'), static_cast<char8_t>('\xed'),
           static_cast<char8_t>('\xbf'), static_cast<char8_t>('\xbf'),
           static_cast<char8_t>('\xed'), static_cast<char8_t>('\xaf'),
           static_cast<char8_t>('A')},
    .output{{std::unexpected{utf_transcoding_error::encoded_surrogate}},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {std::unexpected{utf_transcoding_error::encoded_surrogate}},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {std::unexpected{utf_transcoding_error::encoded_surrogate}},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {U'A'}}};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char32_t> table3_10{
    .input{static_cast<char8_t>('\xf4'), static_cast<char8_t>('\x91'),
           static_cast<char8_t>('\x92'), static_cast<char8_t>('\x93'),
           static_cast<char8_t>('\xff'), static_cast<char8_t>('\x41'),
           static_cast<char8_t>('\x80'), static_cast<char8_t>('\xbf'),
           static_cast<char8_t>('B')},
    .output{{std::unexpected{utf_transcoding_error::out_of_range}},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {std::unexpected{utf_transcoding_error::invalid_utf8_leading_byte}},
            {U'A'},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}},
            {U'B'}}};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char32_t> table3_11{
    .input{static_cast<char8_t>('\xe1'), static_cast<char8_t>('\x80'),
           static_cast<char8_t>('\xe2'), static_cast<char8_t>('\xf0'),
           static_cast<char8_t>('\x91'), static_cast<char8_t>('\x92'),
           static_cast<char8_t>('\xf1'), static_cast<char8_t>('\xbf'),
           static_cast<char8_t>('A')},
    .output{{std::unexpected{utf_transcoding_error::truncated_utf8_sequence}},
            {std::unexpected{utf_transcoding_error::truncated_utf8_sequence}},
            {std::unexpected{utf_transcoding_error::truncated_utf8_sequence}},
            {std::unexpected{utf_transcoding_error::truncated_utf8_sequence}},
            {U'A'}}};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char32_t> two_byte{
    .input{static_cast<char8_t>('\xc2'), static_cast<char8_t>('\xa0')},
    .output{{U'\u00A0'}}};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char32_t> two_byte_truncated_by_end{
    .input{static_cast<char8_t>('\xc2')},
    .output{{std::unexpected{utf_transcoding_error::truncated_utf8_sequence}}}};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char32_t> two_byte_truncated_by_non_continuation{
    .input{static_cast<char8_t>('\xc2'), static_cast<char8_t>('A')},
    .output{{std::unexpected{utf_transcoding_error::truncated_utf8_sequence}},
            {U'A'}}};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char32_t> three_byte{
    .input{static_cast<char8_t>('\xe4'), static_cast<char8_t>('\xba'),
           static_cast<char8_t>('\xba')},
    .output{{U'\u4EBA'}}};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char32_t> three_byte_truncated_at_third_by_end{
    .input{static_cast<char8_t>('\xe4'), static_cast<char8_t>('\xba')},
    .output{{std::unexpected{utf_transcoding_error::truncated_utf8_sequence}}}};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char32_t>
    four_byte_truncated_at_second_by_non_continuation{
        .input{static_cast<char8_t>('\xf0'), static_cast<char8_t>('A')},
        .output{{std::unexpected{utf_transcoding_error::truncated_utf8_sequence}},
                {U'A'}}};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char32_t> four_byte_truncated_at_third_by_end{
    .input{static_cast<char8_t>('\xf0'), static_cast<char8_t>('\x90')},
    .output{{std::unexpected{utf_transcoding_error::truncated_utf8_sequence}}}};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char32_t> four_byte_truncated_at_fourth_by_end{
    .input{static_cast<char8_t>('\xf0'), static_cast<char8_t>('\x90'),
           static_cast<char8_t>('\x80')},
    .output{{std::unexpected{utf_transcoding_error::truncated_utf8_sequence}}}};

CONSTEXPR_UNLESS_MSVC test_case<char16_t, char32_t> single_high{
    .input{u'\xD800'},
    .output{{std::unexpected{utf_transcoding_error::unpaired_high_surrogate}}}};

CONSTEXPR_UNLESS_MSVC test_case<char16_t, char32_t>
    surrogate_pair_truncated_by_non_low_surrogate{
        .input{u'\xD800', u'A'},
        .output{{std::unexpected{utf_transcoding_error::unpaired_high_surrogate}},
                {U'A'}}};

CONSTEXPR_UNLESS_MSVC test_case<char16_t, char32_t> single_low{
    .input{u'\xDC00'},
    .output{{std::unexpected{utf_transcoding_error::unpaired_low_surrogate}}}};

CONSTEXPR_UNLESS_MSVC test_case<char16_t, char32_t> surrogates{
    .input{u'\xD800', u'\xDC00'}, .output{{U'\U00010000'}}};

CONSTEXPR_UNLESS_MSVC test_case<char16_t, char32_t> nonsurrogates{
    .input{u'X', u'Y', u'Z'}, .output{{U'X'}, {U'Y'}, {U'Z'}}};

CONSTEXPR_UNLESS_MSVC test_case<char32_t, char32_t> encoded_surrogate{
    .input{U'\x0000DC00'},
    .output{{std::unexpected{utf_transcoding_error::encoded_surrogate}}}};

CONSTEXPR_UNLESS_MSVC test_case<char32_t, char32_t> out_of_range{
    .input{U'\x00110000'},
    .output{{std::unexpected{utf_transcoding_error::out_of_range}}}};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char8_t> valid_utf8_identity{
    .input{static_cast<char8_t>('A'), static_cast<char8_t>('\xc2'),
           static_cast<char8_t>('\xa0'), static_cast<char8_t>('\xe4'),
           static_cast<char8_t>('\xba'), static_cast<char8_t>('\xba'),
           static_cast<char8_t>('\xf0'), static_cast<char8_t>('\x90'),
           static_cast<char8_t>('\x80'), static_cast<char8_t>('\x80')},
    .output{{static_cast<char8_t>('A')},
            {static_cast<char8_t>('\xc2')},
            {static_cast<char8_t>('\xa0')},
            {static_cast<char8_t>('\xe4')},
            {static_cast<char8_t>('\xba')},
            {static_cast<char8_t>('\xba')},
            {static_cast<char8_t>('\xf0')},
            {static_cast<char8_t>('\x90')},
            {static_cast<char8_t>('\x80')},
            {static_cast<char8_t>('\x80')}}};

CONSTEXPR_UNLESS_MSVC test_case<char, char8_t> valid_utf8_char_identity{
    .input{'A', '\xc2', '\xa0', '\xe4', '\xba', '\xba', '\xf0', '\x90', '\x80', '\x80'},
    .output{{static_cast<char8_t>('A')},
            {static_cast<char8_t>('\xc2')},
            {static_cast<char8_t>('\xa0')},
            {static_cast<char8_t>('\xe4')},
            {static_cast<char8_t>('\xba')},
            {static_cast<char8_t>('\xba')},
            {static_cast<char8_t>('\xf0')},
            {static_cast<char8_t>('\x90')},
            {static_cast<char8_t>('\x80')},
            {static_cast<char8_t>('\x80')}}};

CONSTEXPR_UNLESS_MSVC test_case<char16_t, char16_t> valid_utf16_identity{
    .input{u'A', u'\xD800', u'\xDC00'},
    .output{{u'A'}, {u'\xD800'}, {u'\xDC00'}}};

CONSTEXPR_UNLESS_MSVC test_case<wchar_t, char16_t> valid_utf16_wchar_identity{
    .input{L'A', L'\x4EBA'}, .output{{u'A'}, {u'\x4EBA'}}};

CONSTEXPR_UNLESS_MSVC test_case<char32_t, char32_t> valid_utf32_identity{
    .input{U'X', U'Y', U'Z'}, .output{{U'X'}, {U'Y'}, {U'Z'}}};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char32_t> four_continuations{
    .input{static_cast<char8_t>('\xf0'), static_cast<char8_t>('\x90'),
           static_cast<char8_t>('\x80'), static_cast<char8_t>('\x80'),
           static_cast<char8_t>('\x80')},
    .output{{U'\x00010000'},
            {std::unexpected{utf_transcoding_error::unexpected_utf8_continuation_byte}}}};

CONSTEXPR_UNLESS_MSVC test_case<char16_t, char32_t> two_low_surrogates{
    .input{u'\xD800', u'\xDC00', u'\xDC00'},
    .output{{U'\x00010000'},
            {std::unexpected{utf_transcoding_error::unpaired_low_surrogate}}}};

CONSTEXPR_UNLESS_MSVC test_case<char8_t, char32_t> ff_at_end{
    .input{static_cast<char8_t>('\xc3'), static_cast<char8_t>('\xa9'),
           static_cast<char8_t>('\xff')},
    .output{{U'\u00E9'},
            {std::unexpected{utf_transcoding_error::invalid_utf8_leading_byte}}}};


template <typename WrappingIterator, exposition_only_code_unit_from CharTFrom,
          exposition_only_code_unit_to CharTTo>
constexpr bool run_test_case_impl(test_case<CharTFrom, CharTTo> test_case) {
  auto view{[&] {
    auto it{WrappingIterator(test_case.input)};
    if constexpr (!std::copyable<WrappingIterator>) {
      std::ranges::subrange subrange{std::move(it), std::default_sentinel};
      return std::move(subrange) | to_utf_or_error<CharTTo>;
    } else {
      auto end{WrappingIterator(test_case.input)};
      while (end != std::default_sentinel) {
        ++end;
      }
      std::ranges::subrange subrange{it, end};
      return subrange | to_utf_or_error<CharTTo>;
    }
  }()};
  {
    auto output_it{test_case.output.begin()};
    auto view_it{view.begin()};
    auto end{view.end()};
    do {
      if (*view_it != *output_it) {
        return false;
      }
      ++output_it;
      ++view_it;
    } while (view_it != end);
  }
  if constexpr (std::copyable<WrappingIterator>) {
    auto output_it{test_case.output.begin()};
    auto view_it{view.begin()};
    auto end{view.begin()};
    while (end != view.end()) {
      ++end;
    }
    do {
      if (*view_it != *output_it) {
        return false;
      }
      ++output_it;
      ++view_it;
    } while (view_it != end);
  }
  if constexpr (std::bidirectional_iterator<WrappingIterator>) {
    auto it2{WrappingIterator(test_case.input)};
    auto end2{view.end().base()};
    std::ranges::subrange subrange2{it2, end2};
    auto view2{std::move(subrange2) | to_utf_or_error<CharTTo>};
    {
      auto view_it{view2.end()};
      auto output_it{test_case.output.end()};
      auto end{view2.begin()};
      do {
        --view_it;
        --output_it;
        if (*view_it != *output_it) {
          return false;
        }
      } while (view_it != end);
    }
  }
  return true;
}

template <exposition_only_code_unit_from CharTFrom, exposition_only_code_unit_to CharTTo>
constexpr bool run_test_case(test_case<CharTFrom, CharTTo> test_case) {
  return run_test_case_impl<test_input_iterator<CharTFrom>>(test_case) &&
      run_test_case_impl<test_copyable_input_iterator<CharTFrom>>(test_case) &&
      run_test_case_impl<test_forward_iterator<CharTFrom>>(test_case) &&
      run_test_case_impl<test_bidi_iterator<CharTFrom>>(test_case);
}

template <typename CharT>
constexpr bool input_iterator_test(std::initializer_list<CharT> const single_arr) {
  test_input_iterator single_begin(single_arr);
  std::ranges::subrange subrange{std::move(single_begin), std::default_sentinel};
  auto single_view{to_utf32_view(std::move(subrange))};
  std::u32string single_u32{single_view | std::ranges::to<std::u32string>()};
  if (single_u32.size() != 1 || single_u32.at(0) != U'x') {
    return false;
  }
  return true;
}

template <typename CharT>
constexpr bool forward_iterator_test(std::initializer_list<CharT> const single_arr) {
  test_forward_iterator single_begin(single_arr);
  std::ranges::subrange subrange{std::move(single_begin), std::default_sentinel};
  auto single_view{to_utf32_view(std::move(subrange))};
  std::u32string single_u32{single_view | std::ranges::to<std::u32string>()};
  if (single_u32.size() != 1 || single_u32.at(0) != U'x') {
    return false;
  }
  return true;
}

template <typename CharT>
constexpr bool bidi_iterator_test(std::initializer_list<CharT> const single_arr) {
  test_bidi_iterator single_begin(single_arr);
  std::ranges::subrange subrange{std::move(single_begin), std::default_sentinel};
  auto single_view{to_utf32_view(std::move(subrange))};
  std::u32string single_u32{single_view | std::ranges::to<std::u32string>()};
  if (single_u32.size() != 1 || single_u32.at(0) != U'x') {
    return false;
  }
  return true;
}

template <typename CharT>
constexpr bool double_encode_test(std::initializer_list<CharT> const single_arr) {
  test_forward_iterator single_begin(single_arr);
  std::ranges::subrange subrange{std::move(single_begin), std::default_sentinel};
  auto single_view{to_utf32_view(std::move(subrange))};
  auto double_encoded_view{to_utf8_view(std::move(single_view))};
  std::u8string single_u8{double_encoded_view | std::ranges::to<std::u8string>()};
  if (single_u8.size() != 1 || single_u8.at(0) != U'x') {
    return false;
  }
  return true;
}

constexpr bool utf_iterator_base_test() {
  std::initializer_list<char8_t> arr{u8'b', u8'a', u8'r'};
  {
    test_forward_iterator it_begin(arr);
    std::ranges::subrange subrange{std::move(it_begin), std::default_sentinel};
    auto single_encoded_view = subrange | to_utf16;
    auto iter = single_encoded_view.begin();
    ++iter;
    if (*iter.base() != u8'a') {
      return false;
    }
  }
  {
    test_forward_iterator it_begin(arr);
    std::ranges::subrange subrange{std::move(it_begin), std::default_sentinel};
    auto double_encoded_view = subrange | to_utf16 | to_utf32;
    auto iter = double_encoded_view.begin();
    ++iter;
    if (*iter != U'a') {
      return false;
    }
    if (*iter.base() != u'a') {
      return false;
    }
  }
  {
    test_bidi_iterator it_begin(arr);
    std::ranges::subrange subrange{std::move(it_begin), std::default_sentinel};
    auto double_encoded_view = subrange | to_utf16 | to_utf32;
    auto iter = double_encoded_view.begin();
    ++iter;
    if (*iter != U'a') {
      return false;
    }
    if (*iter.base() != u'a') {
      return false;
    }
  }
  return true;
}

constexpr bool post_increment_decrement_test() {
  std::initializer_list<char8_t> arr{u8'x'};
  {
    test_input_iterator it_begin(arr);
    std::ranges::subrange subrange{std::move(it_begin), std::default_sentinel};
    auto single_encoded_view = std::move(subrange) | to_utf16;
    auto iter = single_encoded_view.begin();
    if (*iter != u'x') {
      return false;
    }
    static_assert(std::is_same_v<void, decltype(iter++)>);
    iter++;
    if (iter != std::default_sentinel) {
      return false;
    }
  }
  {
    test_bidi_iterator it_begin(arr);
    std::ranges::subrange subrange{std::move(it_begin), std::default_sentinel};
    auto single_encoded_view = subrange | to_utf16;
    auto iter = single_encoded_view.begin();
    if (*iter != u'x') {
      return false;
    }
    auto inc_result = iter++;
    if (inc_result != single_encoded_view.begin()) {
      return false;
    }
    if (iter != std::default_sentinel) {
      return false;
    }
    auto dec_result = iter--;
    if (dec_result != std::default_sentinel) {
      return false;
    }
    if (iter != single_encoded_view.begin()) {
      return false;
    }
  }
  return true;
}

constexpr bool to_utf_test() {
  std::ranges::empty_view<char8_t> empty_view{};
  auto empty_utf_view{empty_view | to_utf8};
  static_assert(
      std::is_same_v<decltype(empty_utf_view), std::ranges::empty_view<char8_t>>);
  auto u8_string_literal_utf_view{u8"foo" | to_utf32};
  auto u8_string_literal_utf_view_it{u8_string_literal_utf_view.begin()};
  if (*u8_string_literal_utf_view_it != U'f') {
    return false;
  }
  ++u8_string_literal_utf_view_it;
  if (*u8_string_literal_utf_view_it != U'o') {
    return false;
  }
  ++u8_string_literal_utf_view_it;
  if (*u8_string_literal_utf_view_it != U'o') {
    return false;
  }
  ++u8_string_literal_utf_view_it;
  if (u8_string_literal_utf_view_it != u8_string_literal_utf_view.end()) {
    return false;
  }
  char8_t const non_null_terminated_array[3]{u8'a', u8'b', u8'c'};
  auto non_null_terminated_array_utf_view{non_null_terminated_array | to_utf32};
  auto non_null_terminated_array_utf_view_it{non_null_terminated_array_utf_view.begin()};
  if (*non_null_terminated_array_utf_view_it != U'a') {
    return false;
  }
  ++non_null_terminated_array_utf_view_it;
  if (*non_null_terminated_array_utf_view_it != U'b') {
    return false;
  }
  ++non_null_terminated_array_utf_view_it;
  if (*non_null_terminated_array_utf_view_it != U'c') {
    return false;
  }
  ++non_null_terminated_array_utf_view_it;
  if (non_null_terminated_array_utf_view_it != non_null_terminated_array_utf_view.end()) {
    return false;
  }
  std::initializer_list<char8_t> arr{u8'b', u8'a', u8'r'};
  test_input_iterator input_it(arr);
  std::ranges::subrange subrange{std::move(input_it), std::default_sentinel};
  auto input_utf_view{std::move(subrange) | to_utf32};
  auto input_utf_view_it{std::ranges::begin(input_utf_view)};
  if (*input_utf_view_it != U'b') {
    return false;
  }
  ++input_utf_view_it;
  if (*input_utf_view_it != U'a') {
    return false;
  }
  ++input_utf_view_it;
  if (*input_utf_view_it != U'r') {
    return false;
  }
  ++input_utf_view_it;
  if (input_utf_view_it != input_utf_view.end()) {
    return false;
  }
  char8_t const* u8_ptr{u8"foo"};
  auto u8_ptr_utf_view{null_term(u8_ptr) | to_utf32};
  auto u8_ptr_utf_view_it{u8_ptr_utf_view.begin()};
  if (*u8_ptr_utf_view_it != U'f') {
    return false;
  }
  ++u8_ptr_utf_view_it;
  if (*u8_ptr_utf_view_it != U'o') {
    return false;
  }
  ++u8_ptr_utf_view_it;
  if (*u8_ptr_utf_view_it != U'o') {
    return false;
  }
  ++u8_ptr_utf_view_it;
  if (u8_ptr_utf_view_it != u8_ptr_utf_view.end()) {
    return false;
  }
  return true;
}

template <template <typename> typename TestIterator>
CONSTEXPR_UNLESS_MSVC bool wrapped_view_mid_code_point_test_impl() {
  enum class base_test {
    none,
    range,
    iterator_mid_code_point,
    iterator_full_code_point
  };
  auto const test{[](base_test const base_testing) {
    std::initializer_list<char32_t> nums{U'\x0001D360', U'\x0001D361', U'\x0001D362',
                                         U'\x0001D363', U'\x0001D364', U'\x0001D365',
                                         U'\x0001D366', U'\x0001D367'};
    auto const make_u16_subrange{
        [&]() -> std::optional<std::ranges::subrange<
                  std::ranges::iterator_t<to_utf16_view<std::ranges::subrange<
                      TestIterator<char32_t>, std::default_sentinel_t>>>,
                  std::default_sentinel_t>> {
          TestIterator<char32_t> start_it{nums};
          auto u32_subrange{
              std::ranges::subrange(std::move(start_it), std::default_sentinel)};
          to_utf16_view u16v{std::move(u32_subrange)};
          auto u16_begin{u16v.begin()};
          if (*u16_begin != L'\xD834') {
            return std::nullopt;
          }
          ++u16_begin;
          if (*u16_begin != L'\xDF60') {
            return std::nullopt;
          }
          ++u16_begin;
          if (*u16_begin != L'\xD834') {
            return std::nullopt;
          }
          ++u16_begin;
          if (*u16_begin != L'\xDF61') {
            return std::nullopt;
          }
          auto u16_end{u16v.end()};
          return std::optional{std::ranges::subrange(std::move(u16_begin), u16_end)};
        }};
    auto u16_subrange{make_u16_subrange().value()};
    to_utf8_view u8v{std::move(u16_subrange)};
    if (base_testing == base_test::range) {
      auto u16_subrange2{make_u16_subrange().value()};
      auto u8v_base{std::move(u8v).base()};
      if (u8v_base.begin() != u16_subrange2.begin()) {
        return false;
      }
      return true;
    }
    auto u8_begin{u8v.begin()};
    // todo
    // if (*u8_begin !=
    //     std::unexpected{utf_transcoding_error::unpaired_low_surrogate}) {
    //   return false;
    // }
    if (base_testing == base_test::iterator_mid_code_point) {
      if constexpr (!std::forward_iterator<TestIterator<char32_t>>) {
        if (std::move(u8_begin).base() != ++make_u16_subrange().value().begin()) {
          return false;
        }
      } else {
        if (std::move(u8_begin).base() != make_u16_subrange().value().begin()) {
          return false;
        }
      }
      return true;
    }
    if (*u8_begin != 0xEF) {
      return false;
    }
    ++u8_begin;
    if (*u8_begin != 0xBF) {
      return false;
    }
    ++u8_begin;
    if (*u8_begin != 0xBD) {
      return false;
    }
    ++u8_begin;
    // todo
    // if (!(*u8_begin).has_value()) {
    //   return false;
    // }
    if (base_testing == base_test::iterator_full_code_point) {
      auto expected_base{make_u16_subrange().value().begin()};
      if constexpr (!std::forward_iterator<TestIterator<char32_t>>) {
        std::ranges::advance(expected_base, 3);
      } else {
        std::ranges::advance(expected_base, 1);
      }
      if (std::move(u8_begin).base() != expected_base) {
        return false;
      }
      return true;
    }
    if (*u8_begin != 0xF0) {
      return false;
    }
    auto u8_end{u8v.end()};
    auto u8_subrange{std::ranges::subrange(std::move(u8_begin), u8_end)};
    auto u16v2{std::move(u8_subrange)};
    static_assert(sizeof(u16v2.begin()) > sizeof(u16_subrange.begin()));
    return true;
  }};
  return test(base_test::none) && test(base_test::range) &&
      test(base_test::iterator_mid_code_point) &&
      test(base_test::iterator_full_code_point);
}

CONSTEXPR_UNLESS_MSVC bool wrapped_view_bidirectional_mid_code_point_test_impl() {
  std::initializer_list<char32_t> nums{U'\x0001D360', U'\x0001D361', U'\x0001D362',
                                       U'\x0001D363', U'\x0001D364', U'\x0001D365',
                                       U'\x0001D366', U'\x0001D367'};
  test_bidi_iterator<char32_t> start_it{nums};
  test_bidi_iterator<char32_t> end_it{nums};
  std::ranges::advance(end_it, std::ranges::size(nums));
  auto u32_subrange{std::ranges::subrange(start_it, end_it)};
  to_utf16_view u16v{u32_subrange};
  auto u16_begin{u16v.begin()};
  if (*u16_begin != L'\xD834') {
    return false;
  }
  ++u16_begin;
  if (*u16_begin != L'\xDF60') {
    return false;
  }
  ++u16_begin;
  if (*u16_begin != L'\xD834') {
    return false;
  }
  ++u16_begin;
  if (*u16_begin != L'\xDF61') {
    return false;
  }
  auto u16_end{u16v.end()};
  --u16_end;
  if (*u16_end != L'\xDF67') {
    return false;
  }
  --u16_end;
  if (*u16_end != L'\xD834') {
    return false;
  }
  --u16_end;
  if (*u16_end != L'\xDF66') {
    return false;
  }
  auto u16_subrange{std::ranges::subrange(u16_begin, u16_end)};
  to_utf8_view u8v{u16_subrange};
  if (u8v.base().begin() != u16_subrange.begin()) {
    return false;
  }
  if (u8v.base().end() != u16_subrange.end()) {
    return false;
  }
  auto u8_begin{u8v.begin()};
  if (u8_begin.base() == u16_begin) {
    return false;
  }
  if (*u8_begin != 0xF0) {
    return false;
  }
  ++u8_begin;
  if (*u8_begin != 0x9D) {
    return false;
  }
  auto u8_end{u8v.end()};
  if (u8_end.base() == u16_end) {
    return false;
  }
  if (*u8_end != 0xF0) {
    return false;
  }
  --u8_end;
  if (*u8_end != 0xA5) {
    return false;
  }
  auto u8_subrange{std::ranges::subrange(u8_begin, u8_end)};
  auto u16v2{u8_subrange};
  static_assert(sizeof(u16v2.begin()) == sizeof(u16_begin));
  static_assert(sizeof(u16v2.end()) == sizeof(u16_end));
  return true;
}

template <template <typename> typename TestIterator>
CONSTEXPR_UNLESS_MSVC bool wrapped_view_full_code_point_test_impl() {
  std::initializer_list<char32_t> nums{U'\x0001D360', U'\x0001D361', U'\x0001D362',
                                       U'\x0001D363', U'\x0001D364', U'\x0001D365',
                                       U'\x0001D366', U'\x0001D367'};
  TestIterator<char32_t> start_it{nums};
  TestIterator<char32_t> end_it{nums};
  std::ranges::advance(end_it, std::ranges::size(nums));
  auto u32_subrange{std::ranges::subrange(start_it, end_it)};
  to_utf16_view u16v{u32_subrange};
  auto u16_begin{u16v.begin()};
  if (*u16_begin != L'\xD834') {
    return false;
  }
  ++u16_begin;
  if (*u16_begin != L'\xDF60') {
    return false;
  }
  ++u16_begin;
  if (*u16_begin != L'\xD834') {
    return false;
  }
  auto u16_end{u16v.end()};
  if constexpr (std::bidirectional_iterator<TestIterator<char32_t>>) {
    --u16_end;
    if (*u16_end != L'\xDF67') {
      return false;
    }
    --u16_end;
    if (*u16_end != L'\xD834') {
      return false;
    }
  }
  auto u16_subrange{std::ranges::subrange(u16_begin, u16_end)};
  to_utf8_view u8v{u16_subrange};
  if (u8v.base().begin() != u16_subrange.begin()) {
    return false;
  }
  if (u8v.base().end() != u16_subrange.end()) {
    return false;
  }
  auto u8_begin{u8v.begin()};
  if (u8_begin.base() != u16_begin) {
    return false;
  }
  if (*u8_begin != 0xF0) {
    return false;
  }
  ++u8_begin;
  if (*u8_begin != 0x9D) {
    return false;
  }
  auto u8_end{u8v.end()};
  if constexpr (std::bidirectional_iterator<TestIterator<char32_t>>) {
    if (u8_end.base() != u16_end) {
      return false;
    }
    if (*u8_end != 0xF0) {
      return false;
    }
    --u8_end;
    if (*u8_end != 0xA6) {
      return false;
    }
  } else {
    if (u8_end != u16_end) {
      return false;
    }
  }
  auto u8_subrange{std::ranges::subrange(u8_begin, u8_end)};
  auto u16v2{u8_subrange};
  if constexpr (std::bidirectional_iterator<TestIterator<char32_t>>) {
    static_assert(sizeof(u16v2.begin()) == sizeof(u16_begin));
    static_assert(sizeof(u16v2.end()) == sizeof(u16_end));
  }
  return true;
}

CONSTEXPR_UNLESS_MSVC bool wrapped_view_test() {
  if (!wrapped_view_mid_code_point_test_impl<test_comparable_input_iterator>()) {
    return false;
  }
  if (!wrapped_view_mid_code_point_test_impl<test_forward_iterator>()) {
    return false;
  }
  if (!wrapped_view_bidirectional_mid_code_point_test_impl()) {
    return false;
  }
  if (!wrapped_view_full_code_point_test_impl<test_forward_iterator>()) {
    return false;
  }
  if (!wrapped_view_full_code_point_test_impl<test_bidi_iterator>()) {
    return false;
  }
  return true;
}

constexpr bool empty_test() {
  static_assert(std::is_same_v<decltype(std::views::empty<char8_t> | to_utf8),
                               std::ranges::empty_view<char8_t>>);
  auto empty_utf{to_utf8_view{std::views::empty<char8_t>}};
  if (!empty_utf.empty()) {
    return false;
  }
  return true;
}

CONSTEXPR_UNLESS_MSVC bool rvalue_array_test() {
  auto view = std::array{U'\uFFFD'} | to_utf16;
  if (*std::ranges::begin(view) != u'\uFFFD') {
    return false;
  }
  return true;
}

struct iconv_t { };

// For the sake of simplicity, this iconv only converts between UTF-8 and UTF-32.
size_t iconv([[maybe_unused]] iconv_t cd, const char** inbuf, size_t* inbytesleft,
             char** outbuf, size_t* outbytesleft) {
#if 0
  if (!inbuf) {
    return 0;
  }
  if (inbuf && !*inbuf) {
    return 0;
  }
  assert(inbytesleft);
  assert(outbuf);
  assert(*outbuf);
  assert(outbytesleft);
  auto view = std::ranges::subrange(*inbuf, *inbuf + *inbytesleft) | to_utf32;
  for (auto it = std::ranges::begin(view), end = std::ranges::end(view); it != end;) {
    if (it.success()) {
      if (*outbytesleft < sizeof(char32_t)) {
        errno = E2BIG;
        return static_cast<std::size_t>(-1);
      }
      char32_t c = *it;
      (*outbuf)[0] = static_cast<char>((c >> 24) & 0xFF);
      (*outbuf)[1] = static_cast<char>((c >> 16) & 0xFF);
      (*outbuf)[2] = static_cast<char>((c >> 8) & 0xFF);
      (*outbuf)[3] = static_cast<char>(c & 0xFF);
      *outbuf += sizeof(char32_t);
      *outbytesleft -= sizeof(char32_t);
      ++it;
      std::size_t bytes_converted = it.base() - *inbuf;
      *inbytesleft -= bytes_converted;
      *inbuf = it.base();
    } else {
      utf_transcoding_error e = it.success().error();
      switch (e) {
      case utf_transcoding_error::truncated_utf8_sequence: {
        errno = EINVAL;
      } break;
      case utf_transcoding_error::unexpected_utf8_continuation_byte:
      case utf_transcoding_error::overlong:
      case utf_transcoding_error::encoded_surrogate:
      case utf_transcoding_error::out_of_range:
      case utf_transcoding_error::invalid_utf8_leading_byte: {
        errno = EILSEQ;
      } break;
      case utf_transcoding_error::unpaired_high_surrogate:
      case utf_transcoding_error::unpaired_low_surrogate: {
        std::unreachable();
      }
      }
      return static_cast<std::size_t>(-1);
    }
  }
  return 0;
#else
  throw;
#endif
}

bool iconv_test() {
  {
    std::array const input{'\xc3', '\xa9'};
    std::array output{'\0', '\0', '\0', '\0'};
    char const* inbuf{input.data()};
    char* outbuf{output.data()};
    std::size_t inbytesleft{sizeof(input)};
    std::size_t outbytesleft{sizeof(output)};
    std::size_t result{iconv(iconv_t{}, &inbuf, &inbytesleft, &outbuf, &outbytesleft)};
    if (!(result == 0 &&
          output == std::array{'\0', '\0', '\0', '\xe9'} && inbytesleft == 0 &&
          outbytesleft == 0)) {
      return false;
    }
  }
  {
    std::array const input{'\xc3', '\xa9', '\xc3', '\xa9'};
    std::array output{'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
    char const* inbuf{input.data()};
    char* outbuf{output.data()};
    std::size_t inbytesleft{sizeof(input)};
    std::size_t outbytesleft{sizeof(output)};
    std::size_t result{iconv(iconv_t{}, &inbuf, &inbytesleft, &outbuf, &outbytesleft)};
    if (!(result == 0 &&
          output ==
              std::array<char, 8>{'\0', '\0', '\0', '\xe9', '\0', '\0', '\0', '\xe9'} &&
          inbytesleft == 0 && outbytesleft == 0)) {
      return false;
    }
  }
  {
    std::array const input{'\xc3', '\xa9', '\xc2'};
    std::array output{'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
    char const* inbuf{input.data()};
    char* outbuf{output.data()};
    std::size_t inbytesleft{sizeof(input)};
    std::size_t outbytesleft{sizeof(output)};
    std::size_t result{iconv(iconv_t{}, &inbuf, &inbytesleft, &outbuf, &outbytesleft)};
    if (!(result == static_cast<std::size_t>(-1) &&
          output == std::array{'\0', '\0', '\0', '\xe9', '\0', '\0', '\0', '\0'} &&
          inbytesleft == 1 && outbytesleft == 4 && errno == EINVAL)) {
      return false;
    }
  }
  {
    std::array const input{'\xc3', '\xa9', '\xe0', '\x80'};
    std::array output{'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
    char const* inbuf{input.data()};
    char* outbuf{output.data()};
    std::size_t inbytesleft{sizeof(input)};
    std::size_t outbytesleft{sizeof(output)};
    std::size_t result{iconv(iconv_t{}, &inbuf, &inbytesleft, &outbuf, &outbytesleft)};
    if (!(result == static_cast<std::size_t>(-1) &&
          output ==
              std::array<char, 8>{'\0', '\0', '\0', '\xe9', '\0', '\0', '\0', '\0'} &&
          inbytesleft == 2 && outbytesleft == 4 && errno == EILSEQ)) {
      return false;
    }
  }
  return true;
}

enum UErrorCode {
  U_ZERO_ERROR = 0,
  U_ILLEGAL_ARGUMENT_ERROR,
  U_INVALID_CHAR_FOUND
};

constexpr char16_t* u_strFromUTF8WithSub(char16_t* dest, int32_t destCapacity,
                                         int32_t* pDestLength, const char* src,
                                         int32_t srcLength, char32_t subchar,
                                         int32_t* pNumSubstitutions,
                                         UErrorCode* pErrorCode) {
#if 0
  if (*pErrorCode != U_ZERO_ERROR) {
    return nullptr;
  }
  if ((src == nullptr && srcLength != 0) || srcLength < -1 || (destCapacity < 0) ||
      (dest == nullptr && destCapacity > 0) || subchar > 0x10ffff ||
      (0xD800 <= subchar && subchar <= 0xDFFF)) {
    *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
    return nullptr;
  }

  if (pNumSubstitutions != nullptr) {
    *pNumSubstitutions = 0;
  }

  auto impl = [&](auto view) {
    auto end = std::ranges::end(view);
    if (pDestLength) {
      *pDestLength = 0;
      for (auto it = std::ranges::begin(view); it != end; ++it) {
        *pDestLength += it.success() ? 1 : (subchar > 0xFFFF ? 2 : 1);
      }
    }
    if (destCapacity == 0) {
      return dest;
    }
    char16_t* out_ptr = dest;
    for (auto it = std::ranges::begin(view); it != end; ++it) {
      auto write = [&](char16_t c) {
        *out_ptr = c;
        ++out_ptr;
        --destCapacity;
      };
      if (it.success()) {
        if (destCapacity == 0) {
          return dest;
        }
        write(*it);
      } else {
        if (subchar == static_cast<char32_t>(-1)) {
          *pErrorCode = U_INVALID_CHAR_FOUND;
          return dest;
        } else {
          ++*pNumSubstitutions;
          if (subchar > 0xFFFF) {
            std::array<char16_t, 2> subchar_utf16{};
            std::ranges::copy(std::array{subchar} | to_utf16, subchar_utf16.data());
            write(subchar_utf16[0]);
            if (destCapacity == 0) {
              return dest;
            }
            write(subchar_utf16[1]);
          } else {
            write(static_cast<char16_t>(subchar));
          }
        }
      }
    }
    if (destCapacity > 0) {
      *out_ptr = char16_t{};
    }
    return dest;
  };

  if (srcLength == -1) {
    return impl(null_term(src) | to_utf16);
  } else {
    return impl(std::ranges::subrange(src, src + srcLength) | to_utf16);
  }
#else
  throw;
#endif
}

constexpr bool u_strFromUTF8WithSub_test() {
  auto test = [](bool const use_nullterm, bool const preflight) {
    {
      std::array const input{'\xc3', '\xa9', '\0'};
      std::array output{u'\xFFFF', u'\xFFFF'};
      int32_t destLength{};
      int32_t numSubstitutions{};
      UErrorCode errorCode{};
      char16_t* result{
          u_strFromUTF8WithSub(output.data(), preflight ? 0 : output.size(), &destLength,
                               input.data(), use_nullterm ? -1 : input.size() - 1,
                               U'\x0001F574', &numSubstitutions, &errorCode)};
      if (!((preflight || output == std::array{u'\u00E9', u'\0'}) &&
            (use_nullterm || destLength == 1) && (preflight || numSubstitutions == 0) &&
            errorCode == U_ZERO_ERROR && result == output.data())) {
        return false;
      }
    }
    {
      std::array const input{'\xc3', '\xa9', '\xc3', '\xa9', '\0'};
      std::array output{u'\xFFFF', u'\xFFFF', u'\xFFFF'};
      int32_t destLength{};
      int32_t numSubstitutions{};
      UErrorCode errorCode{};
      char16_t* result{
          u_strFromUTF8WithSub(output.data(), preflight ? 0 : output.size(), &destLength,
                               input.data(), use_nullterm ? -1 : input.size() - 1,
                               U'\x0001F574', &numSubstitutions, &errorCode)};
      if (!((preflight || output == std::array{u'\u00E9', u'\u00E9', u'\0'}) &&
            (use_nullterm || destLength == 2) && (preflight || numSubstitutions == 0) &&
            errorCode == U_ZERO_ERROR && result == output.data())) {
        return false;
      }
    }
    {
      std::array const input{'\xc3', '\xa9', '\xc2', '\0'};
      std::array output{u'\xFFFF', u'\xFFFF', u'\xFFFF', u'\xFFFF'};
      int32_t destLength{};
      int32_t numSubstitutions{};
      UErrorCode errorCode{};
      char16_t* result{
          u_strFromUTF8WithSub(output.data(), preflight ? 0 : output.size(), &destLength,
                               input.data(), use_nullterm ? -1 : input.size() - 1,
                               U'\x0001F574', &numSubstitutions, &errorCode)};
      if (!((preflight || output == std::array{u'\u00E9', u'\xD83D', u'\xDD74', u'\0'}) &&
            (use_nullterm || destLength == 3) && (preflight || numSubstitutions == 1) &&
            errorCode == U_ZERO_ERROR && result == output.data())) {
        return false;
      }
    }
    {
      std::array const input{'\xc3', '\xa9', '\xe0', '\x80', '\0'};
      std::array output{u'\xFFFF', u'\xFFFF', u'\xFFFF', u'\xFFFF', u'\xFFFF', u'\xFFFF'};
      int32_t destLength{};
      int32_t numSubstitutions{};
      UErrorCode errorCode{};
      char16_t* result{
          u_strFromUTF8WithSub(output.data(), preflight ? 0 : output.size(), &destLength,
                               input.data(), use_nullterm ? -1 : input.size() - 1,
                               U'\x0001F574', &numSubstitutions, &errorCode)};
      if (!((preflight ||
             output ==
                 std::array{u'\u00E9', u'\xD83D', u'\xDD74', u'\xD83D', u'\xDD74',
                            u'\0'}) &&
            (use_nullterm || destLength == 5) && (preflight || numSubstitutions == 2) &&
            errorCode == U_ZERO_ERROR && result == output.data())) {
        return false;
      }
    }
    return true;
  };
  if (!test(true, true)) {
    return false;
  }
  if (!test(true, false)) {
    return false;
  }
  if (!test(false, true)) {
    return false;
  }
  if (!test(false, false)) {
    return false;
  }
  return true;
}

inline constexpr unsigned long MB_ERR_INVALID_CHARS = 1;
inline constexpr unsigned long ERROR_INSUFFICIENT_BUFFER = 1;
inline constexpr unsigned long ERROR_INVALID_FLAGS = 2;
inline constexpr unsigned long ERROR_INVALID_PARAMETER = 3;
inline constexpr unsigned long ERROR_NO_UNICODE_TRANSLATION = 4;
inline constexpr unsigned int CP_UTF8 = 0;

CONSTEXPR_UNLESS_MSVC void SetLastError(unsigned long dwErrCode) {
  (void)dwErrCode;
}

template <bool WindowsXp>
CONSTEXPR_UNLESS_MSVC int MultiByteToWideChar(unsigned int CodePage,
                                              unsigned long dwFlags,
                                              const char* lpMultiByteStr, int cbMultiByte,
                                              wchar_t* lpWideCharStr, int cchWideChar) {
#if 0
  (void)CodePage; // For simplicity we only implement CP_UTF8
  auto impl = [&](auto view) {
    auto end = std::ranges::end(view);
    if (cchWideChar == 0) {
      if constexpr (WindowsXp) {
        int chars = 0;
        for (auto it = std::ranges::begin(view); it != end; ++it) {
          chars += it.success() ? 1 : 0;
        }
        return chars;
      } else {
        return static_cast<int>(std::ranges::distance(view));
      }
    } else {
      wchar_t* out_ptr = lpWideCharStr;
      for (auto it = std::ranges::begin(view); it != end; ++it) {
        auto write = [&](auto c) {
          *out_ptr = static_cast<wchar_t>(c);
          ++out_ptr;
          --cchWideChar;
        };
        if (it.success()) {
          if (cchWideChar == 0) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return 0;
          }
          write(*it);
        } else {
          if (dwFlags == MB_ERR_INVALID_CHARS) {
            SetLastError(ERROR_NO_UNICODE_TRANSLATION);
            return 0;
          }
          if constexpr (!WindowsXp) {
            if (cchWideChar == 0) {
              SetLastError(ERROR_INSUFFICIENT_BUFFER);
              return 0;
            }
            write(*it);
          }
        }
      }
      return static_cast<int>(out_ptr - lpWideCharStr);
    }
  };
  if (cbMultiByte == -1) {
    if constexpr (sizeof(wchar_t) == 2) {
      return impl(null_term(lpMultiByteStr) | to_utf16);
    } else {
      return impl(null_term(lpMultiByteStr) | to_utf32);
    }
  } else {
    if constexpr (sizeof(wchar_t) == 2) {
      return impl(std::ranges::subrange(lpMultiByteStr, lpMultiByteStr + cbMultiByte) |
                  to_utf16);
    } else {
      return impl(std::ranges::subrange(lpMultiByteStr, lpMultiByteStr + cbMultiByte) |
                  to_utf32);
    }
  }
#else
  throw;
#endif
}

template <bool WindowsXp>
CONSTEXPR_UNLESS_MSVC bool MultiByteToWideChar_test() {
  auto test = [](bool const use_nullterm, bool const preflight,
                 bool const fail_on_invalid) {
    {
      std::array const input{'\xc3', '\xa9', '\0'};
      std::array output{L'\0'};
      int result{MultiByteToWideChar<WindowsXp>(
          CP_UTF8, fail_on_invalid ? MB_ERR_INVALID_CHARS : 0, input.data(),
          use_nullterm ? -1 : input.size() - 1, output.data(),
          preflight ? 0 : output.size())};
      if (!((preflight || output == std::array{L'\u00E9'}) && result == 1)) {
        return false;
      }
    }
    {
      std::array const input{'\xc3', '\xa9', '\xc3', '\xa9', '\0'};
      std::array output{L'\0', L'\0'};
      int result{MultiByteToWideChar<WindowsXp>(
          CP_UTF8, fail_on_invalid ? MB_ERR_INVALID_CHARS : 0, input.data(),
          use_nullterm ? -1 : input.size() - 1, output.data(),
          preflight ? 0 : output.size())};
      if (!((preflight || output == std::array{L'\u00E9', L'\u00E9'}) && result == 2)) {
        return false;
      }
    }
    {
      std::array const input{'\xc3', '\xa9', '\xc2', '\0'};
      std::array output{L'\0', L'\0'};
      int result{MultiByteToWideChar<WindowsXp>(
          CP_UTF8, fail_on_invalid ? MB_ERR_INVALID_CHARS : 0, input.data(),
          use_nullterm ? -1 : input.size() - 1, output.data(),
          preflight ? 0 : output.size())};
      auto expected_output{[&] {
        if (WindowsXp || fail_on_invalid) {
          return std::array{L'\u00E9', L'\0'};
        } else {
          return std::array{L'\u00E9', L'\uFFFD'};
        }
      }()};
      if (!((preflight || output == expected_output) &&
            (result == 2 || (WindowsXp && result == 1) ||
             (fail_on_invalid && result == 0)))) {
        return false;
      }
    }
    {
      std::array const input{'\xc3', '\xa9', '\xe0', '\x80', '\0'};
      std::array output{L'\0', L'\0', L'\0'};
      int result{MultiByteToWideChar<WindowsXp>(
          CP_UTF8, fail_on_invalid ? MB_ERR_INVALID_CHARS : 0, input.data(),
          use_nullterm ? -1 : input.size() - 1, output.data(),
          preflight ? 0 : output.size())};
      auto expected_output{[&] {
        if (WindowsXp || fail_on_invalid) {
          return std::array{L'\u00E9', L'\0', L'\0'};
        } else {
          return std::array{L'\u00E9', L'\uFFFD', L'\uFFFD'};
        }
      }()};
      if (!((preflight || output == expected_output) &&
            (result == 3 || (WindowsXp && result == 1) ||
             (fail_on_invalid && result == 0)))) {
        return false;
      }
    }
    return true;
  };
  if (!test(true, true, true)) {
    return false;
  }
  if (!test(true, true, false)) {
    return false;
  }
  if (!test(true, false, true)) {
    return false;
  }
  if (!test(true, false, false)) {
    return false;
  }
  if (!test(false, true, true)) {
    return false;
  }
  if (!test(false, true, false)) {
    return false;
  }
  if (!test(false, false, true)) {
    return false;
  }
  if (!test(false, false, false)) {
    return false;
  }
  return true;
}

template <typename FromChar, typename ToChar>
std::basic_string<ToChar> decode(std::basic_string_view<FromChar> input) {
#if 0
  std::basic_string<ToChar> result;
  result.reserve(input.size()); // like what size_hint does
  auto view = input | to_utf<ToChar>;
  for (auto it = std::ranges::begin(view), end = std::ranges::end(view); it != end;
       ++it) {
    if (it.success()) {
      result.push_back(*it);
    } else {
      auto pos_curr = it.base() - input.begin();
      auto it2 = it;
      auto pos_next = (++it2).base() - input.begin();
      std::ostringstream ss;
      ss << "can't decode ";
      if (pos_next > pos_curr + 1) {
        ss << "characters";
      } else {
        ss << "character 0x" << std::hex
           << static_cast<unsigned int>(static_cast<unsigned char>(*it.base()))
           << std::dec;
      }
      ss << " in position " << pos_curr;
      if (pos_next > pos_curr + 1) {
        ss << "-" << pos_next - 1;
      }
      ss << ": ";
      ss << [&] {
        switch (it.success().error()) {
        case utf_transcoding_error::truncated_utf8_sequence:
          return "unexpected end of data";
        case utf_transcoding_error::unpaired_high_surrogate:
        case utf_transcoding_error::unpaired_low_surrogate:
          return "illegal UTF-16 surrogate";
        case utf_transcoding_error::unexpected_utf8_continuation_byte:
        case utf_transcoding_error::invalid_utf8_leading_byte:
          return "invalid start byte";
        case utf_transcoding_error::encoded_surrogate:
          if constexpr (std::same_as<FromChar, char32_t>) {
            return "code point in surrogate code point range(0xd800, 0xe000)";
          }
        case utf_transcoding_error::overlong:
          if constexpr (std::same_as<FromChar, char32_t>) {
            return "code point not in range(0x110000)";
          }
        case utf_transcoding_error::out_of_range:
          return "invalid continuation byte";
        }
        std::unreachable();
      }();
      throw std::runtime_error(std::move(ss).str());
    }
  }
  return result;
#else
  throw;
#endif
}

bool decode_test() {
  auto check_exception_what = [](auto func, std::string const& what) {
    try {
      func();
    } catch (std::exception const& ex) {
      return ex.what() == what;
    }
    return false;
  };
  if (decode<char, char32_t>("\xc3\xa9") != U"\u00E9") {
    return false;
  }
  if (decode<char, char32_t>("\xc3\xa9\xc3\xa9") != U"\u00E9\u00E9") {
    return false;
  }
  if (!check_exception_what(
          [] { decode<char, char32_t>("\xc3\xa9\xff"); },
          "can't decode character 0xff in position 2: invalid start byte")) {
    return false;
  }
  if (!check_exception_what(
          [] { decode<char, char32_t>("\xc3\xa9\xe1\x80"); },
          "can't decode characters in position 2-3: unexpected end of data")) {
    return false;
  }
  return true;
}

static_assert(std::ranges::borrowed_range<to_utf8_view<std::string_view>>);
static_assert(std::ranges::borrowed_range<to_utf16_view<std::string_view>>);
static_assert(std::ranges::borrowed_range<to_utf32_view<std::string_view>>);

CONSTEXPR_UNLESS_MSVC bool utf_view_test() {
  if (!input_iterator_test(std::initializer_list<char8_t>{u8'x'})) {
    return false;
  }
  if (!forward_iterator_test(std::initializer_list<char8_t>{u8'x'})) {
    return false;
  }
  if (!bidi_iterator_test(std::initializer_list<char8_t>{u8'x'})) {
    return false;
  }
  if (!double_encode_test(std::initializer_list<char8_t>{u8'x'})) {
    return false;
  }
  if (!input_iterator_test(std::initializer_list<char16_t>{u'x'})) {
    return false;
  }
  if (!forward_iterator_test(std::initializer_list<char16_t>{u'x'})) {
    return false;
  }
  if (!bidi_iterator_test(std::initializer_list<char16_t>{u'x'})) {
    return false;
  }
  if (!double_encode_test(std::initializer_list<char16_t>{u'x'})) {
    return false;
  }
  if (!input_iterator_test(std::initializer_list<char32_t>{U'x'})) {
    return false;
  }
  if (!forward_iterator_test(std::initializer_list<char32_t>{U'x'})) {
    return false;
  }
  if (!bidi_iterator_test(std::initializer_list<char32_t>{U'x'})) {
    return false;
  }
  if (!double_encode_test(std::initializer_list<char32_t>{U'x'})) {
    return false;
  }
  if (!run_test_case(table3_8)) {
    return false;
  }
  if (!run_test_case(table3_9)) {
    return false;
  }
  if (!run_test_case(table3_10)) {
    return false;
  }
  if (!run_test_case(table3_11)) {
    return false;
  }
  if (!run_test_case(two_byte)) {
    return false;
  }
  if (!run_test_case(two_byte_truncated_by_end)) {
    return false;
  }
  if (!run_test_case(two_byte_truncated_by_non_continuation)) {
    return false;
  }
  if (!run_test_case(three_byte)) {
    return false;
  }
  if (!run_test_case(three_byte_truncated_at_third_by_end)) {
    return false;
  }
  if (!run_test_case(four_byte_truncated_at_second_by_non_continuation)) {
    return false;
  }
  if (!run_test_case(four_byte_truncated_at_third_by_end)) {
    return false;
  }
  if (!run_test_case(four_byte_truncated_at_fourth_by_end)) {
    return false;
  }
  if (!run_test_case(single_high)) {
    return false;
  }
  if (!run_test_case(surrogate_pair_truncated_by_non_low_surrogate)) {
    return false;
  }
  if (!run_test_case(single_low)) {
    return false;
  }
  if (!run_test_case(surrogates)) {
    return false;
  }
  if (!run_test_case(nonsurrogates)) {
    return false;
  }
  if (!run_test_case(encoded_surrogate)) {
    return false;
  }
  if (!run_test_case(out_of_range)) {
    return false;
  }
  if (!run_test_case(valid_utf8_identity)) {
    return false;
  }
  if (!run_test_case(valid_utf8_char_identity)) {
    return false;
  }
  if (!run_test_case(valid_utf16_identity)) {
    return false;
  }
  if (!run_test_case(valid_utf16_wchar_identity)) {
    return false;
  }
  if (!run_test_case(valid_utf32_identity)) {
    return false;
  }
  if (!run_test_case(four_continuations)) {
    return false;
  }
  if (!run_test_case(two_low_surrogates)) {
    return false;
  }
  if (!run_test_case(ff_at_end)) {
    return false;
  }
  if (!utf_iterator_base_test()) {
    return false;
  }
  if (!post_increment_decrement_test()) {
    return false;
  }
  if (!to_utf_test()) {
    return false;
  }
  if (!wrapped_view_test()) {
    return false;
  }
  if (!empty_test()) {
    return false;
  }
  if (!rvalue_array_test()) {
    return false;
  }
  return true;
}

bool utf_view_appendix_tests() {
  if (!iconv_test()) {
    return false;
  }
  if (!decode_test()) {
    return false;
  }
  return true;
}

CONSTEXPR_UNLESS_MSVC bool utf_view_constexpr_appendix_tests() {
  if (!u_strFromUTF8WithSub_test()) {
    return false;
  }
  if (!MultiByteToWideChar_test<true>()) {
    return false;
  }
  if (!MultiByteToWideChar_test<false>()) {
    return false;
  }
  return true;
}

#if 0
#ifndef _MSC_VER
static_assert(utf_view_test());
static_assert(utf_view_constexpr_appendix_tests());
#endif
#endif

static auto const init{[] {
  framework::tests().insert({"utf_view_test", &utf_view_test});
  // framework::tests().insert(
  //     {"utf_view_constexpr_appendix_tests", &utf_view_constexpr_appendix_tests});
  // framework::tests().insert({"utf_view_appendix_tests", &utf_view_appendix_tests});
  struct {
  } result{};
  return result;
}()};

} // namespace utf_view::tests
