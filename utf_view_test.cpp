module;

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <string>

#ifdef _MSC_VER
#define P2728_CONSTEXPR // https://developercommunity.visualstudio.com/t/std::initializer_list-data-member-of-cla/10622889
#else
#define P2728_CONSTEXPR constexpr
#endif

export module p2728:utf_view_test;

import :utf_view;

namespace p2728::utf_view_test {

  template<class CharT>
  struct test_input_iterator {
    using value_type = CharT;
    using reference_type = CharT const&;
    using difference_type = std::ptrdiff_t;
    using iterator_concept = std::input_iterator_tag;
    constexpr explicit test_input_iterator(std::initializer_list<CharT> const& list) : begin{list.begin()}, end{list.end()} { }
    test_input_iterator(test_input_iterator const&) = delete;
    test_input_iterator& operator=(test_input_iterator const&) = delete;
    test_input_iterator(test_input_iterator&&) = default;
    test_input_iterator& operator=(test_input_iterator&&) = default;
    constexpr reference_type operator*() const { return *begin; }
    constexpr test_input_iterator& operator++() {
      ++begin;
      return *this;
    }
    constexpr void operator++(int) { ++begin; }

    friend constexpr bool operator==(std::default_sentinel_t const&, test_input_iterator const& rhs) {
      return rhs.begin == rhs.end;
    }

    std::initializer_list<CharT>::iterator begin;
    std::initializer_list<CharT>::iterator end;
  };

  template<class CharT>
  test_input_iterator(std::initializer_list<CharT>) -> test_input_iterator<CharT>;

  static_assert(std::input_iterator<test_input_iterator<char8_t>>);

  template<class CharT>
  struct test_forward_iterator {
    static constexpr std::initializer_list<CharT> empty{};

    using value_type = CharT;
    using reference_type = CharT const&;
    using difference_type = std::ptrdiff_t;
    using iterator_concept = std::forward_iterator_tag;
    constexpr test_forward_iterator() : begin{empty.begin()}, end{empty.end()} { }
    constexpr explicit test_forward_iterator(std::initializer_list<CharT> const& list) : begin{list.begin()}, end{list.end()} { }
    test_forward_iterator(test_forward_iterator const&) = default;
    test_forward_iterator& operator=(test_forward_iterator const&) = default;
    test_forward_iterator(test_forward_iterator&&) = default;
    test_forward_iterator& operator=(test_forward_iterator&&) = default;
    constexpr reference_type operator*() const { return *begin; }
    constexpr test_forward_iterator& operator++() {
      ++begin;
      return *this;
    }
    constexpr test_forward_iterator operator++(int) {
      auto ret = *this;
      ++begin;
      return ret;
    }

    friend constexpr bool operator==(std::default_sentinel_t const&, test_forward_iterator const& rhs) {
      return rhs.begin == rhs.end;
    }

    friend constexpr bool operator==(test_forward_iterator const& lhs, test_forward_iterator const& rhs) {
      return lhs.begin == rhs.begin && lhs.end == rhs.end;
    }

    std::initializer_list<CharT>::iterator begin;
    std::initializer_list<CharT>::iterator end;
  };

  template<class CharT>
  test_forward_iterator(std::initializer_list<CharT>) -> test_forward_iterator<CharT>;

  static_assert(std::forward_iterator<test_forward_iterator<char8_t>>);

  template<class CharT>
  struct test_bidi_iterator {
    static constexpr std::initializer_list<CharT> empty{};

    using value_type = CharT;
    using reference_type = CharT const&;
    using difference_type = std::ptrdiff_t;
    using iterator_concept = std::bidirectional_iterator_tag;
    constexpr test_bidi_iterator() : begin{empty.begin()}, end{empty.end()} { }
    constexpr explicit test_bidi_iterator(std::initializer_list<CharT> const& list) : begin{list.begin()}, end{list.end()} { }
    test_bidi_iterator(test_bidi_iterator const&) = default;
    test_bidi_iterator& operator=(test_bidi_iterator const&) = default;
    test_bidi_iterator(test_bidi_iterator&&) = default;
    test_bidi_iterator& operator=(test_bidi_iterator&&) = default;
    constexpr reference_type operator*() const { return *begin; }
    constexpr test_bidi_iterator& operator++() {
      ++begin;
      return *this;
    }
    constexpr test_bidi_iterator operator++(int) {
      auto ret = *this;
      ++begin;
      return ret;
    }
    constexpr test_bidi_iterator& operator--() {
      --begin;
      return *this;
    }
    constexpr test_bidi_iterator operator--(int) {
      auto ret = *this;
      --begin;
      return ret;
    }

    friend constexpr bool operator==(std::default_sentinel_t const&, test_bidi_iterator const& rhs) {
      return rhs.begin == rhs.end;
    }
    friend constexpr bool operator==(test_bidi_iterator const& lhs, test_bidi_iterator const& rhs) {
      return lhs.begin == rhs.begin && lhs.end == rhs.end;
    }

    std::initializer_list<CharT>::iterator begin;
    std::initializer_list<CharT>::iterator end;
  };

  template<class CharT>
  test_bidi_iterator(std::initializer_list<CharT>) -> test_bidi_iterator<CharT>;

  static_assert(std::bidirectional_iterator<test_bidi_iterator<char8_t>>);

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

  P2728_CONSTEXPR test_case<char8_t, char32_t> table3_8{.input{(char8_t)'\xc0', (char8_t)'\xaf', (char8_t)'\xe0', (char8_t)'\x80',
                                                               (char8_t)'\xbf', (char8_t)'\xf0', (char8_t)'\x81', (char8_t)'\x82',
                                                               (char8_t)'A'},
                                                        .output{{U'\uFFFD', {transcoding_error::invalid}},
                                                                {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                {U'\uFFFD', {transcoding_error::overlong}},
                                                                {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                {U'\uFFFD', {transcoding_error::overlong}},
                                                                {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                {U'A', {}}}};

  P2728_CONSTEXPR test_case<char8_t, char32_t> table3_9{.input{(char8_t)'\xed', (char8_t)'\xa0', (char8_t)'\x80', (char8_t)'\xed',
                                                               (char8_t)'\xbf', (char8_t)'\xbf', (char8_t)'\xed', (char8_t)'\xaf',
                                                               (char8_t)'A'},
                                                        .output{{U'\uFFFD', {transcoding_error::encoded_surrogate}},
                                                                {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                {U'\uFFFD', {transcoding_error::encoded_surrogate}},
                                                                {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                {U'\uFFFD', {transcoding_error::encoded_surrogate}},
                                                                {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                {U'A', {}}}};

  P2728_CONSTEXPR test_case<char8_t, char32_t> table3_10{.input{(char8_t)'\xf4', (char8_t)'\x91', (char8_t)'\x92',
                                                                (char8_t)'\x93', (char8_t)'\xff', (char8_t)'\x41',
                                                                (char8_t)'\x80', (char8_t)'\xbf', (char8_t)'B'},
                                                         .output{{U'\uFFFD', {transcoding_error::out_of_range}},
                                                                 {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                 {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                 {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                 {U'\uFFFD', {transcoding_error::invalid}},
                                                                 {U'A', {}},
                                                                 {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                 {U'\uFFFD', {transcoding_error::bad_continuation_or_surrogate}},
                                                                 {U'B', {}}}};

  P2728_CONSTEXPR test_case<char8_t, char32_t> table3_11{.input{(char8_t)'\xe1', (char8_t)'\x80', (char8_t)'\xe2',
                                                                (char8_t)'\xf0', (char8_t)'\x91', (char8_t)'\x92',
                                                                (char8_t)'\xf1', (char8_t)'\xbf', (char8_t)'A'},
                                                         .output{{U'\uFFFD', {transcoding_error::truncated}},
                                                                 {U'\uFFFD', {transcoding_error::truncated}},
                                                                 {U'\uFFFD', {transcoding_error::truncated}},
                                                                 {U'\uFFFD', {transcoding_error::truncated}},
                                                                 {U'A', {}}}};

  template<typename WrappingIterator, EOcode_unitOE CharTFrom, EOcode_unitOE CharTTo>
  constexpr bool run_test_case_impl(test_case<CharTFrom, CharTTo> test_case) {
    auto it{WrappingIterator(test_case.input)};
    std::ranges::subrange subrange{std::move(it), std::default_sentinel};
    utf_view<CharTTo, decltype(subrange)> view{std::move(subrange)};
    {
      auto output_it{test_case.output.begin()};
      auto view_it{view.begin()};
      auto end{view.end()};
      do {
        if (*view_it != output_it->code_unit) {
          return false;
        }
        if (view_it.error() != output_it->error) {
          return false;
        }
        ++output_it;
        ++view_it;
      } while (view_it != end);
    }
    if constexpr (std::bidirectional_iterator<decltype(it)>) {
      auto it2{WrappingIterator(test_case.input)};
      auto end2{it2};
      while (end2 != std::default_sentinel) {
        ++end2;
      }
      std::ranges::subrange subrange2{it2, end2};
      utf_view<CharTTo, decltype(subrange2)> view2{std::move(subrange2)};
      {
        auto view_it{view2.end()};
        auto output_it{test_case.output.end()};
        auto end{view2.begin()};
        do {
          --view_it;
          --output_it;
          if (*view_it != output_it->code_unit) {
            return false;
          }
          if (view_it.error() != output_it->error) {
            return false;
          }
        } while (view_it != end);
      }
    }
    return true;
  }

  template<EOcode_unitOE CharTFrom, EOcode_unitOE CharTTo>
  constexpr bool run_test_case(test_case<CharTFrom, CharTTo> test_case) {
    return run_test_case_impl<test_input_iterator<CharTFrom>>(test_case) &&
      run_test_case_impl<test_forward_iterator<CharTFrom>>(test_case) &&
      run_test_case_impl<test_bidi_iterator<CharTFrom>>(test_case);
  }

  template <typename CharT>
  constexpr bool input_iterator_test(std::initializer_list<CharT> const single_arr) {
    test_input_iterator single_begin(single_arr);
    std::ranges::subrange subrange{std::move(single_begin), std::default_sentinel};
    utf_view<char32_t, decltype(subrange)> single_view{std::move(subrange)};
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
    utf_view<char32_t, decltype(subrange)> single_view{std::move(subrange)};
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
    utf_view<char32_t, decltype(subrange)> single_view{std::move(subrange)};
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
    utf_view<char32_t, decltype(subrange)> single_view{std::move(subrange)};
    utf_view<char8_t, decltype(single_view)> double_encoded_view{std::move(single_view)};
    std::u8string single_u8{double_encoded_view | std::ranges::to<std::u8string>()};
    if (single_u8.size() != 1 || single_u8.at(0) != U'x') {
      return false;
    }
    return true;
  }

  export P2728_CONSTEXPR bool utf_view_test() {
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

#ifndef _MSC_VER
  static_assert(utf_view_test());
#endif

  // GCC bug workaround
  export bool utf_view_test2() {
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
