module;

#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <ranges>
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

    friend constexpr bool operator==(std::default_sentinel_t const&, test_input_iterator const& rhs) { return rhs.ptr == rhs.end; }

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
    constexpr explicit test_forward_iterator(std::array<CharT, Size> const& arr) : ptr{arr.data()}, end{arr.data() + arr.size()} { }
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

    friend constexpr bool operator==(std::default_sentinel_t const&, test_forward_iterator const& rhs) { return rhs.ptr == rhs.end; }

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

  template <EOcode_unitOE CharT>
  struct test_case_code_unit_result {
    CharT code_unit;
    std::optional<transcoding_error> error;
  };

  template <EOcode_unitOE CharTFrom, std::size_t FromSize, EOcode_unitOE CharTTo, std::size_t ToSize>
  struct test_case {
    std::array<CharTFrom, FromSize> input;
    std::array<test_case_code_unit_result<CharTTo>, ToSize> output;
  };

  constexpr test_case<char8_t, 9, char32_t, 9> table3_8{
    .input{u8'\xc0', u8'\xaf', u8'\xe0', u8'\x80', u8'\xbf', u8'\xf0', u8'\x81', u8'\x82', u8'A'},
    .output{{{U'\uFFFD', {transcoding_error::overlong}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'\uFFFD', {transcoding_error::overlong}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'\uFFFD', {transcoding_error::overlong}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'A', {}}}}};

  constexpr test_case<char8_t, 9, char32_t, 9> table3_9{
    .input{u8'\xed', u8'\xa0', u8'\x80', u8'\xed', u8'\xbf', u8'\xbf', u8'\xed', u8'\xaf', u8'A'},
    .output{{{U'\uFFFD', {transcoding_error::surrogate}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'\uFFFD', {transcoding_error::surrogate}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'\uFFFD', {transcoding_error::surrogate}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'A', {}}}}};

  constexpr test_case<char8_t, 9, char32_t, 9> table3_10{
    .input{u8'\xf4', u8'\x91', u8'\x92', u8'\x93', u8'\xff', u8'\x41', u8'\x80', u8'\xbf', u8'B'},
    .output{{{U'\uFFFD', {transcoding_error::out_of_range}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'\uFFFD', {transcoding_error::out_of_range}},
             {U'A', {}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'\uFFFD', {transcoding_error::unexpected_continuation}},
             {U'B', {}}}}};

  constexpr test_case<char8_t, 9, char32_t, 5> table3_11{
    .input{u8'\xe1', u8'\x80', u8'\xe2', u8'\xf0', u8'\x91', u8'\x92', u8'\xf1', u8'\xbf', u8'A'},
    .output{{{U'\uFFFD', {transcoding_error::truncated}},
             {U'\uFFFD', {transcoding_error::truncated}},
             {U'\uFFFD', {transcoding_error::truncated}},
             {U'\uFFFD', {transcoding_error::truncated}},
             {U'A', {}}}}};


  template <EOcode_unitOE CharTFrom, std::size_t FromSize, EOcode_unitOE CharTTo, std::size_t ToSize>
  constexpr bool run_test_case(test_case<CharTFrom, FromSize, CharTTo, ToSize> test_case) {
    std::ranges::subrange subrange{test_case.input.begin(), test_case.input.end()};
    utf_view<CharTTo, decltype(subrange)> view{subrange};
    for (auto view_it{view.begin()}, output_it{test_case.output.begin()}, end{view.end()}; view_it != end; ++view_it, ++output_it) {
      if (*view_it != output_it->code_unit) {
        return false;
      }
      if (view_it.error() != output_it->error) {
        return false;
      }
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

  static_assert(utf_view_test());

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
