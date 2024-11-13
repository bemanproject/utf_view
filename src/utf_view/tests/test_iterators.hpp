// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef UTF_VIEW_TESTS_TEST_ITERATORS_HPP
#define UTF_VIEW_TESTS_TEST_ITERATORS_HPP

namespace utf_view::tests {

template <class CharT>
struct test_input_iterator {
  using value_type = CharT;
  using reference_type = CharT const&;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::input_iterator_tag;
  constexpr explicit test_input_iterator(std::initializer_list<CharT> const& list)
      : begin{list.begin()},
        end{list.end()} { }
  test_input_iterator(test_input_iterator const&) = delete;
  test_input_iterator& operator=(test_input_iterator const&) = delete;
  test_input_iterator(test_input_iterator&&) = default;
  test_input_iterator& operator=(test_input_iterator&&) = default;
  constexpr reference_type operator*() const {
    return *begin;
  }
  constexpr test_input_iterator& operator++() {
    ++begin;
    return *this;
  }
  constexpr void operator++(int) {
    ++begin;
  }

  friend constexpr bool operator==(std::default_sentinel_t const&,
                                   test_input_iterator const& rhs) {
    return rhs.begin == rhs.end;
  }

  std::initializer_list<CharT>::iterator begin;
  std::initializer_list<CharT>::iterator end;
};

template <class CharT>
test_input_iterator(std::initializer_list<CharT>) -> test_input_iterator<CharT>;

static_assert(std::input_iterator<test_input_iterator<char8_t>>);

template <class CharT>
struct test_comparable_input_iterator {
  using value_type = CharT;
  using reference_type = CharT const&;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::input_iterator_tag;
  constexpr explicit test_comparable_input_iterator(
      std::initializer_list<CharT> const& list)
      : begin{list.begin()},
        end{list.end()} { }
  test_comparable_input_iterator(test_comparable_input_iterator const&) = delete;
  test_comparable_input_iterator& operator=(test_comparable_input_iterator const&) =
      delete;
  test_comparable_input_iterator(test_comparable_input_iterator&&) = default;
  test_comparable_input_iterator& operator=(test_comparable_input_iterator&&) = default;
  constexpr reference_type operator*() const {
    return *begin;
  }
  constexpr test_comparable_input_iterator& operator++() {
    ++begin;
    return *this;
  }
  constexpr void operator++(int) {
    ++begin;
  }

  friend constexpr bool operator==(std::default_sentinel_t const&,
                                   test_comparable_input_iterator const& rhs) {
    return rhs.begin == rhs.end;
  }

  friend constexpr bool operator==(test_comparable_input_iterator const&,
                                   test_comparable_input_iterator const&) = default;

  std::initializer_list<CharT>::iterator begin;
  std::initializer_list<CharT>::iterator end;
};

template <class CharT>
test_comparable_input_iterator(std::initializer_list<CharT>)
    -> test_comparable_input_iterator<CharT>;

static_assert(std::input_iterator<test_comparable_input_iterator<char8_t>>);

template <class CharT>
struct test_copyable_input_iterator {
  static constexpr std::initializer_list<CharT> empty{};

  using value_type = CharT;
  using reference_type = CharT const&;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::input_iterator_tag;
  constexpr test_copyable_input_iterator()
      : begin{empty.begin()},
        end{empty.end()} { }
  constexpr explicit test_copyable_input_iterator(
      std::initializer_list<CharT> const& list)
      : begin{list.begin()},
        end{list.end()} { }
  test_copyable_input_iterator(test_copyable_input_iterator const&) = default;
  test_copyable_input_iterator& operator=(test_copyable_input_iterator const&) = default;
  test_copyable_input_iterator(test_copyable_input_iterator&&) = default;
  test_copyable_input_iterator& operator=(test_copyable_input_iterator&&) = default;
  constexpr reference_type operator*() const {
    return *begin;
  }
  constexpr test_copyable_input_iterator& operator++() {
    ++begin;
    return *this;
  }
  constexpr void operator++(int) {
    ++begin;
  }

  friend constexpr bool operator==(std::default_sentinel_t const&,
                                   test_copyable_input_iterator const& rhs) {
    return rhs.begin == rhs.end;
  }

  friend constexpr bool operator==(test_copyable_input_iterator const&,
                                   test_copyable_input_iterator const&) = default;

  std::initializer_list<CharT>::iterator begin;
  std::initializer_list<CharT>::iterator end;
};

template <class CharT>
test_copyable_input_iterator(std::initializer_list<CharT>)
    -> test_copyable_input_iterator<CharT>;

static_assert(std::input_iterator<test_copyable_input_iterator<char8_t>>);
static_assert(!std::forward_iterator<test_copyable_input_iterator<char8_t>>);

template <class CharT>
struct test_forward_iterator {
  static constexpr std::initializer_list<CharT> empty{};

  using value_type = CharT;
  using reference_type = CharT const&;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::forward_iterator_tag;
  constexpr test_forward_iterator()
      : begin{empty.begin()},
        end{empty.end()} { }
  constexpr explicit test_forward_iterator(std::initializer_list<CharT> const& list)
      : begin{list.begin()},
        end{list.end()} { }
  test_forward_iterator(test_forward_iterator const&) = default;
  test_forward_iterator& operator=(test_forward_iterator const&) = default;
  test_forward_iterator(test_forward_iterator&&) = default;
  test_forward_iterator& operator=(test_forward_iterator&&) = default;
  constexpr reference_type operator*() const {
    return *begin;
  }
  constexpr test_forward_iterator& operator++() {
    ++begin;
    return *this;
  }
  constexpr test_forward_iterator operator++(int) {
    auto ret = *this;
    ++begin;
    return ret;
  }

  friend constexpr bool operator==(std::default_sentinel_t const&,
                                   test_forward_iterator const& rhs) {
    return rhs.begin == rhs.end;
  }

  friend constexpr bool operator==(test_forward_iterator const&,
                                   test_forward_iterator const&) = default;

  std::initializer_list<CharT>::iterator begin;
  std::initializer_list<CharT>::iterator end;
};

template <class CharT>
test_forward_iterator(std::initializer_list<CharT>) -> test_forward_iterator<CharT>;

static_assert(std::forward_iterator<test_forward_iterator<char8_t>>);

template <class CharT>
struct test_bidi_iterator {
  static constexpr std::initializer_list<CharT> empty{};

  using value_type = CharT;
  using reference_type = CharT const&;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::bidirectional_iterator_tag;
  constexpr test_bidi_iterator()
      : begin{empty.begin()},
        end{empty.end()} { }
  constexpr explicit test_bidi_iterator(std::initializer_list<CharT> const& list)
      : begin{list.begin()},
        end{list.end()} { }
  test_bidi_iterator(test_bidi_iterator const&) = default;
  test_bidi_iterator& operator=(test_bidi_iterator const&) = default;
  test_bidi_iterator(test_bidi_iterator&&) = default;
  test_bidi_iterator& operator=(test_bidi_iterator&&) = default;
  constexpr reference_type operator*() const {
    return *begin;
  }
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

  friend constexpr bool operator==(std::default_sentinel_t const&,
                                   test_bidi_iterator const& rhs) {
    return rhs.begin == rhs.end;
  }
  friend constexpr bool operator==(test_bidi_iterator const&,
                                   test_bidi_iterator const&) = default;

  std::initializer_list<CharT>::iterator begin;
  std::initializer_list<CharT>::iterator end;
};

template <class CharT>
test_bidi_iterator(std::initializer_list<CharT>) -> test_bidi_iterator<CharT>;

static_assert(std::bidirectional_iterator<test_bidi_iterator<char8_t>>);

} // namespace utf_view::tests

#endif // UTF_VIEW_TESTS_TEST_ITERATORS_HPP
