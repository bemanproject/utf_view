module;

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <ranges>
#include <sstream>
#include <string>

export module p2728:utf_view_test;

import :utf_view;

namespace p2728::utf_view_test {

  template<class CharT, std::size_t Size>
  struct test_input_iterator {
    using value_type = CharT;
    using reference_type = CharT const&;
    using difference_type = std::ptrdiff_t;
    using iterator_concept = std::input_iterator_tag;
    constexpr explicit test_input_iterator(std::array<CharT, Size> const& arr) : ptr{arr.data()}, end{arr.data() + arr.size()} { }
    test_input_iterator(test_input_iterator const&) = delete;
    test_input_iterator& operator=(test_input_iterator const&) = delete;
    test_input_iterator(test_input_iterator&&) = default;
    test_input_iterator& operator=(test_input_iterator&&) = default;
    constexpr reference_type operator*() const { return *ptr; }
    constexpr test_input_iterator& operator++() {
      ++ptr;
      return *this;
    }
    constexpr void operator++(int) { ++ptr; }

    friend constexpr bool operator==(std::default_sentinel_t const&, test_input_iterator const& rhs) {
      return rhs.ptr == rhs.end;
    }

    CharT const* ptr;
    CharT const* end;
  };

  template<class CharT, std::size_t Size>
  test_input_iterator(std::array<CharT, Size>) -> test_input_iterator<CharT, Size>;

  template<class CharT, std::size_t Size>
  struct test_forward_iterator {
    using value_type = CharT;
    using reference_type = CharT const&;
    using difference_type = std::ptrdiff_t;
    using iterator_concept = std::input_iterator_tag;
    constexpr explicit test_forward_iterator(std::array<CharT, Size> const& arr) :
        ptr{arr.data()}, end{arr.data() + arr.size()} { }
    test_forward_iterator(test_forward_iterator const&) = default;
    test_forward_iterator& operator=(test_forward_iterator const&) = default;
    test_forward_iterator(test_forward_iterator&&) = default;
    test_forward_iterator& operator=(test_forward_iterator&&) = default;
    constexpr reference_type operator*() const { return *ptr; }
    constexpr test_forward_iterator& operator++() {
      ++ptr;
      return *this;
    }
    constexpr void operator++(int) { ++ptr; }

    friend constexpr bool operator==(std::default_sentinel_t const&, test_forward_iterator const& rhs) {
      return rhs.ptr == rhs.end;
    }

    CharT const* ptr;
    CharT const* end;
  };

  template<class CharT, std::size_t Size>
  test_forward_iterator(std::array<CharT, Size>) -> test_forward_iterator<CharT, Size>;

  template<class CharT, std::size_t Size>
  struct test_bidi_iterator {
    using value_type = CharT;
    using reference_type = CharT const&;
    using difference_type = std::ptrdiff_t;
    using iterator_concept = std::input_iterator_tag;
    constexpr explicit test_bidi_iterator(std::array<CharT, Size> const& arr) : ptr{arr.data()}, end{arr.data() + arr.size()} { }
    test_bidi_iterator(test_bidi_iterator const&) = default;
    test_bidi_iterator& operator=(test_bidi_iterator const&) = default;
    test_bidi_iterator(test_bidi_iterator&&) = default;
    test_bidi_iterator& operator=(test_bidi_iterator&&) = default;
    constexpr reference_type operator*() const { return *ptr; }
    constexpr test_bidi_iterator& operator++() {
      ++ptr;
      return *this;
    }
    constexpr void operator++(int) { ++ptr; }

    friend constexpr bool operator==(std::default_sentinel_t const&, test_bidi_iterator const& rhs) { return rhs.ptr == rhs.end; }

    CharT const* ptr;
    CharT const* end;
  };

  template<class CharT, std::size_t Size>
  test_bidi_iterator(std::array<CharT, Size>) -> test_bidi_iterator<CharT, Size>;

  template<EOcode_unitOE CharT>
  struct test_case_code_unit_result {
    CharT code_unit;
    std::optional<transcoding_error> error;
  };

  template<EOcode_unitOE CharTFrom, EOcode_unitOE CharTTo>
  struct test_case {
    std::initializer_list<CharTFrom> input;
    std::initializer_list<test_case_code_unit_result<CharTTo>> output;
  };

  constexpr test_case<char8_t, char32_t> table3_8{
    .input{u8'\xc0', u8'\xaf', u8'\xe0', u8'\x80', u8'\xbf', u8'\xf0', u8'\x81', u8'\x82', u8'A'},
    .output{{U'\uFFFD', {transcoding_error::overlong}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'\uFFFD', {transcoding_error::overlong}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'\uFFFD', {transcoding_error::overlong}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'A', {}}}};

  constexpr test_case<char8_t, char32_t> table3_9{
    .input{u8'\xed', u8'\xa0', u8'\x80', u8'\xed', u8'\xbf', u8'\xbf', u8'\xed', u8'\xaf', u8'A'},
    .output{{U'\uFFFD', {transcoding_error::surrogate}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'\uFFFD', {transcoding_error::surrogate}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'\uFFFD', {transcoding_error::surrogate}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'A', {}}}};

  constexpr test_case<char8_t, char32_t> table3_10{
    .input{u8'\xf4', u8'\x91', u8'\x92', u8'\x93', u8'\xff', u8'\x41', u8'\x80', u8'\xbf', u8'B'},
    .output{{U'\uFFFD', {transcoding_error::out_of_range}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'\uFFFD', {transcoding_error::out_of_range}},
            {U'A', {}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'\uFFFD', {transcoding_error::unexpected_continuation}},
            {U'B', {}}}};

  constexpr test_case<char8_t, char32_t> table3_11{
    .input{u8'\xe1', u8'\x80', u8'\xe2', u8'\xf0', u8'\x91', u8'\x92', u8'\xf1', u8'\xbf', u8'A'},
    .output{{U'\uFFFD', {transcoding_error::truncated}},
            {U'\uFFFD', {transcoding_error::truncated}},
            {U'\uFFFD', {transcoding_error::truncated}},
            {U'\uFFFD', {transcoding_error::truncated}},
            {U'A', {}}}};

  template<typename T>
  std::string print_char(T c) {
    std::ostringstream os{};
    os << "0x" << std::hex << std::uppercase << (unsigned)c << " ";
    return std::move(os).str();
  }

  template<typename T>
  std::string print_err(T e) {
    std::ostringstream os{};
    os << (e ? (int)*e : -1);
    return std::move(os).str();
  }

  template<EOcode_unitOE CharTFrom, EOcode_unitOE CharTTo>
  constexpr bool run_test_case(test_case<CharTFrom, CharTTo> test_case) {
    std::ranges::for_each(test_case.input, [](auto c) { std::cout << print_char(c); });
    std::cout << std::endl;
    std::ranges::subrange subrange{test_case.input.begin(), test_case.input.end()};
    utf_view<CharTTo, decltype(subrange)> view{subrange};
    for (auto view_it{view.begin()}, output_it{test_case.output.begin()}, input_it{test_case.input.begin()}, end{view.end()};
         view_it != end; ++view_it, ++output_it, ++input_it) {
      std::cout << "for:      " << print_char(*input_it) << std::endl;
      std::cout << "expected: " << print_char(output_it->code_unit) << ' ' << print_err(output_it->error) << std::endl;
      std::cout << "saw:      " << print_char(*view_it) << ' ' << print_err(view_it.error()) << std::endl;
      if (*view_it != output_it->code_unit) {
        return false;
      }
      if (view_it.error() != output_it->error) {
        return false;
      }
    }
    std::cout << "reversed: " << '\n';
    auto reversed_input{subrange | std::views::reverse};
    std::ranges::for_each(reversed_input, [](auto c) { std::cout << print_char(c); });
    std::cout << std::endl;
    utf_view<CharTTo, decltype(reversed_input)> rview{reversed_input};
    auto routput{test_case.output | std::views::reverse};
    {
      auto view_it{rview.end()};
      auto output_it{routput.end()};
      auto input_it{reversed_input.end()};
      auto end{rview.begin()};
      do {
        --view_it;
        --output_it;
        --input_it;
        std::cout << "for:      " << print_char(*input_it) << std::endl;
        std::cout << "expected: " << print_char(output_it->code_unit) << ' ' << print_err(output_it->error) << std::endl;
        std::cout << "saw:      " << print_char(*view_it) << ' ' << print_err(view_it.error()) << std::endl;
        if (*view_it != output_it->code_unit) {
          return false;
        }
        if (view_it.error() != output_it->error) {
          return false;
        }
      } while (view_it != end);
    }
    return true;
  }

  constexpr bool input_iterator_test() {
    std::array<char8_t, 1> const single_arr{u8'x'};
    test_input_iterator single_begin{single_arr};
    std::ranges::subrange subrange{std::move(single_begin), std::default_sentinel};
    utf_view<char32_t, decltype(subrange)> single_view{std::move(subrange)};
    std::u32string single_u32{single_view | std::ranges::to<std::u32string>()};
    if (single_u32.size() != 1 || single_u32.at(0) != U'x') {
      return false;
    }
    return true;
  }

  constexpr bool forward_iterator_test() {
    std::array<char8_t, 1> const single_arr{u8'x'};
    test_forward_iterator single_begin{single_arr};
    std::ranges::subrange subrange{std::move(single_begin), std::default_sentinel};
    utf_view<char32_t, decltype(subrange)> single_view{std::move(subrange)};
    std::u32string single_u32{single_view | std::ranges::to<std::u32string>()};
    if (single_u32.size() != 1 || single_u32.at(0) != U'x') {
      return false;
    }
    return true;
  }

  constexpr bool bidi_iterator_test() {
    std::array<char8_t, 1> const single_arr{u8'x'};
    test_forward_iterator single_begin{single_arr};
    std::ranges::subrange subrange{std::move(single_begin), std::default_sentinel};
    utf_view<char32_t, decltype(subrange)> single_view{std::move(subrange)};
    std::u32string single_u32{single_view | std::ranges::to<std::u32string>()};
    if (single_u32.size() != 1 || single_u32.at(0) != U'x') {
      return false;
    }
    return true;
  }

  export constexpr bool utf_view_test() {
    if (!input_iterator_test()) {
      return false;
    }
    if (!forward_iterator_test()) {
      return false;
    }
    if (!bidi_iterator_test()) {
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
    return true;
  }

  // static_assert(utf_view_test());

  // GCC bug workaround
  export bool utf_view_test2() {
    if (!input_iterator_test()) {
      return false;
    }
    if (!forward_iterator_test()) {
      return false;
    }
    if (!bidi_iterator_test()) {
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
    return true;
  }

} // namespace p2728::utf_view_test
