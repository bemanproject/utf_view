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

template <typename CharT, std::size_t Size>
struct test_input_iterator {
  using value_type = CharT;
  using reference_type = CharT const&;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::input_iterator_tag;
  constexpr explicit test_input_iterator(std::array<CharT, Size> const& arr)
    : ptr{arr.data()},
      end{arr.data() + arr.size()}
  {}
  test_input_iterator(test_input_iterator const&) = delete;
  test_input_iterator& operator=(
    test_input_iterator const&) = delete;
  test_input_iterator(test_input_iterator&&) = default;
  test_input_iterator& operator=(
      test_input_iterator&&) = default;
  constexpr reference_type operator*() const {
    return *ptr;
  }
  constexpr test_input_iterator& operator++() {
    ++ptr;
    return *this;
  }
  constexpr void operator++(int) {
    ++ptr;
  }

  friend constexpr bool operator==(std::default_sentinel_t const&, auto const& rhs) {
    return rhs.ptr == rhs.end;
  }

  CharT const* ptr;
  CharT const* end;
};

template <typename CharT, std::size_t Size>
test_input_iterator(std::array<CharT, Size>) -> test_input_iterator<CharT, Size>;

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

export constexpr bool utf_view_test() {
  if (!input_iterator_test()) {
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
  return true;
}

} // namespace p2728::utf_view_test
