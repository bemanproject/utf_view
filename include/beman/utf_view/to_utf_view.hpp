// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef BEMAN_UTF_VIEW_TO_UTF_VIEW_HPP
#define BEMAN_UTF_VIEW_TO_UTF_VIEW_HPP

#include <beman/utf_view/detail/concepts.hpp>
#include <beman/utf_view/detail/constexpr_unless_msvc.hpp>
#include <beman/utf_view/detail/fake_inplace_vector.hpp>
#include <beman/utf_view/detail/nontype_t_polyfill.hpp>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <expected>
#include <iterator>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>

namespace beman::utf_view {

namespace detail {

  constexpr bool in(unsigned char lo, unsigned char c, unsigned char hi) {
    return lo <= c && c <= hi;
  }

  constexpr bool lead_code_unit(char8_t c) {
    return static_cast<std::uint8_t>(c - 0xc2) <= 0x32;
  }

  constexpr bool is_ascii(char8_t c) {
    return static_cast<std::uint8_t>(c) < 0x80;
  }

  constexpr bool continuation(char8_t c) {
    return static_cast<std::int8_t>(c) < -0x40;
  }

  constexpr int utf8_code_units(char8_t first_unit) {
    return first_unit <= 0x7f ? 1
        : lead_code_unit(first_unit)
        ? static_cast<int>(0xe0 <= first_unit) + static_cast<int>(0xf0 <= first_unit) + 2
        : -1;
  }

  constexpr bool high_surrogate(char16_t c) {
    return 0xD800 <= c && c <= 0xDBFF;
  }

  constexpr bool low_surrogate(char16_t c) {
    return 0xDC00 <= c && c <= 0xDFFF;
  }

  template<typename T>
  struct iter_category_impl {};

  template<std::ranges::forward_range R>
  struct iter_category_impl<R> {
    static consteval auto impl() {
      using category = typename std::iterator_traits<std::ranges::iterator_t<R>>::iterator_category;
      if constexpr (std::derived_from<category, std::bidirectional_iterator_tag>) {
        return std::bidirectional_iterator_tag{};
      } else if constexpr (std::derived_from<category, std::forward_iterator_tag>) {
        return std::forward_iterator_tag{};
      } else {
        return category{};
      }
    }
    using iterator_category = decltype(impl());
  };

} // namespace detail

/* PAPER: namespace std::ranges { */

/* PAPER */

enum class utf_transcoding_error {
  truncated_utf8_sequence,
  unpaired_high_surrogate,
  unpaired_low_surrogate,
  unexpected_utf8_continuation_byte,
  overlong,
  encoded_surrogate,
  out_of_range,
  invalid_utf8_leading_byte
};

enum class to_utf_view_error_kind : bool {
  replacement,
  expected
};

template <exposition_only_code_unit ToType>
struct to_utf_tag_t {
  explicit to_utf_tag_t() = default;
};

template <exposition_only_code_unit ToType>
constexpr to_utf_tag_t<ToType> to_utf_tag{};

using to_utf8_tag_t = to_utf_tag_t<char8_t>;

constexpr to_utf8_tag_t to_utf8_tag{};

using to_utf16_tag_t = to_utf_tag_t<char16_t>;

constexpr to_utf16_tag_t to_utf16_tag{};

using to_utf32_tag_t = to_utf_tag_t<char32_t>;

constexpr to_utf32_tag_t to_utf32_tag{};

template <std::ranges::input_range V, to_utf_view_error_kind E, exposition_only_code_unit ToType>
  requires std::ranges::view<V> && exposition_only_code_unit<std::ranges::range_value_t<V>>
class to_utf_view {
private:
  template<bool> struct exposition_only_iterator; // @*exposition only*@
  template<bool> struct exposition_only_sentinel; // @*exposition only*@

  V base_ = V(); // @*exposition only*@

/* !PAPER */
  template <typename V2>
  struct maybe_cache_begin_impl {
    constexpr auto begin_impl(to_utf_view& self, auto base) noexcept {
      return exposition_only_iterator<false>{self, std::move(base)};
    }
  };

  template <typename V2>
  requires (!std::same_as<std::ranges::range_value_t<V2>, char32_t> && std::copy_constructible<V>)
  struct maybe_cache_begin_impl<V2> {
    constexpr auto begin_impl(to_utf_view& self, auto base) noexcept {
      if (!cache_) {
        cache_.emplace(self, std::move(base));
      }
      return *cache_;
    }
    std::optional<exposition_only_iterator<false>> cache_;
  };

  [[no_unique_address]] maybe_cache_begin_impl<V> cache_;

/* PAPER */
public:
  constexpr to_utf_view()
    requires std::default_initializable<V>
  = default;
  /* PAPER:     constexpr explicit to_utf_view(V base); */
  /* !PAPER */
  constexpr explicit to_utf_view(V base, detail::nontype_t<E> error, to_utf_tag_t<ToType>)
      : base_(std::move(base)) { }
  /* PAPER */

  constexpr V base() const&
    requires std::copy_constructible<V>
  {
    return base_;
  }
  constexpr V base() && {
    return std::move(base_);
  }

  /* !PAPER */
  /* PAPER:     constexpr @*iterator*@<false> begin(); */
  constexpr exposition_only_iterator<false> begin()
  {
    if (!std::copy_constructible<V>) {
      return exposition_only_iterator<false>(*this, std::ranges::begin(base_));
    } else {
      return cache_.begin_impl(*this, std::ranges::begin(base_));
    }
  }
  /* PAPER */
  constexpr exposition_only_iterator<true> begin() const
    requires std::ranges::range<const V> &&
             ((std::same_as<std::ranges::range_value_t<V>, char32_t>) ||
              (!std::ranges::forward_range<const V>)) {
    return exposition_only_iterator<true>(*this, std::ranges::begin(base_));
  }

  constexpr exposition_only_sentinel<false> end()
  {
    return exposition_only_sentinel<false>(std::ranges::end(base_));
  }
  constexpr exposition_only_iterator<false> end() requires std::ranges::common_range<V>
  {
    return exposition_only_iterator<false>(*this, std::ranges::end(base_));
  }
  constexpr exposition_only_sentinel<true> end() const
    requires std::ranges::range<const V> {
    return exposition_only_sentinel<true>(std::ranges::end(base_));
  }
  constexpr exposition_only_iterator<true> end() const
    requires std::ranges::common_range<const V> {
    return exposition_only_iterator<true>(*this, std::ranges::end(base_));
  }

  constexpr bool empty() const {
    return std::ranges::empty(base_);
  }

  constexpr std::size_t size()
    requires std::ranges::sized_range<V> &&
             std::same_as<char32_t, std::ranges::range_value_t<V>> &&
             std::same_as<char32_t, ToType> {
    return std::ranges::size(base_);
  }

  /* !PAPER */
#if defined(__cpp_lib_ranges_reserve_hint)
  /* PAPER:   constexpr auto reserve_hint() requires approximately_sized_range<V>; */
  constexpr auto reserve_hint() requires std::ranges::approximately_sized_range<V> {
    return std::ranges::reserve_hint(base_);
  }
  /* PAPER:   constexpr auto reserve_hint() const requires approximately_sized_range<const V>; */
  constexpr auto reserve_hint() const requires std::ranges::approximately_sized_range<const V>; {
    return std::ranges::reserve_hint(base_);
  }
#endif
  /* PAPER */
};

template <class R, to_utf_view_error_kind E, exposition_only_code_unit ToType>
to_utf_view(R&&, detail::nontype_t<E>, to_utf_tag_t<ToType>) -> to_utf_view<std::views::all_t<R>, E, ToType>;

template <std::ranges::input_range V, to_utf_view_error_kind E, exposition_only_code_unit ToType>
  requires std::ranges::view<V> && exposition_only_code_unit<std::ranges::range_value_t<V>>
template <bool Const>
/* PAPER:   class to_utf_view<V, E, ToType>::@*iterator*@ { */
/* !PAPER */
struct to_utf_view<V, E, ToType>::exposition_only_iterator : detail::iter_category_impl<V> {
/* PAPER */
private:
  using exposition_only_Parent = exposition_only_maybe_const<Const, to_utf_view>; // @*exposition only*@
  using exposition_only_Base = exposition_only_maybe_const<Const, V>; // @*exposition only*@

/* !PAPER */
  using from_type = std::ranges::range_value_t<V>; // @*exposition only*@

  static consteval auto iter_concept_impl() {
    if constexpr (std::ranges::bidirectional_range<exposition_only_Base>) {
      return std::bidirectional_iterator_tag{};
    } else if constexpr (std::ranges::forward_range<exposition_only_Base>) {
      return std::forward_iterator_tag{};
    } else if constexpr (std::ranges::input_range<exposition_only_Base>) {
      return std::input_iterator_tag{};
    }
  }

/* PAPER */
public:
  /* PAPER:   using iterator_concept = @*see below*@; */
  /* PAPER:   using iterator_category = @*see below*@;  // @*not always present*@ */
  /* !PAPER */
  using iterator_concept = decltype(iter_concept_impl());
  /* PAPER */
  using value_type =
      std::conditional_t<E == to_utf_view_error_kind::expected, std::expected<ToType, utf_transcoding_error>, ToType>;
  using reference_type = value_type;
  using difference_type = ptrdiff_t;

private:
/* !PAPER */
#ifdef _MSC_VER
public: // MSVC has some bug with their implementation of friendship
#endif
/* PAPER */
  std::ranges::iterator_t<exposition_only_Base> current_ = std::ranges::iterator_t<exposition_only_Base>(); // @*exposition only*@
  exposition_only_Parent* parent_ = nullptr; // @*exposition only*@

  detail::fake_inplace_vector<value_type, 4 / sizeof(ToType)> buf_{}; // @*exposition only*@

  std::int8_t buf_index_ = 0; // @*exposition only*@
  std::uint8_t to_increment_ = 0; // @*exposition only*@

  /* !PAPER */
  std::expected<void, utf_transcoding_error> success_{};

  /* PAPER */

  template <std::ranges::input_range V2, to_utf_view_error_kind E2, exposition_only_code_unit ToType2>
    requires std::ranges::view<V2> && exposition_only_code_unit<std::ranges::range_value_t<V2>>
  friend class to_utf_view; // @*exposition only*@

public:

  CONSTEXPR_UNLESS_MSVC exposition_only_iterator()
    requires std::default_initializable<std::ranges::iterator_t<V>>
  = default;
  constexpr exposition_only_iterator(
      exposition_only_Parent& parent, std::ranges::iterator_t<exposition_only_Base> begin)
    : current_(std::move(begin)),
      parent_(std::addressof(parent))
  {
    if (base() != exposition_only_end())
      exposition_only_read();
    else if constexpr (!std::ranges::forward_range<exposition_only_Base>) {
      buf_index_ = -1;
    }
  }


  constexpr const std::ranges::iterator_t<exposition_only_Base>& base() const& noexcept {
    return current_;
  }

  constexpr std::ranges::iterator_t<exposition_only_Base> base() && {
    return std::move(current_);
  }

  /* PAPER:       constexpr value_type operator*() const; */
  /* !PAPER */
  constexpr value_type operator*() const {
    if constexpr (E == to_utf_view_error_kind::expected) {
      if (!success_.has_value()) {
        return std::unexpected{success_.error()};
      }
    }
    return buf_[buf_index_];
  }
  /* PAPER */

  constexpr exposition_only_iterator& operator++() requires(E == to_utf_view_error_kind::expected)
  {
    if (!exposition_only_success()) {
      /* !PAPER */
      assert(buf_index_ == 0);
      /* PAPER */
      if constexpr (std::is_same_v<ToType, char8_t>) {
        exposition_only_advance_one();
        exposition_only_advance_one();
      }
    }
    exposition_only_advance_one();
    return *this;
  }

  constexpr exposition_only_iterator& operator++() requires(E == to_utf_view_error_kind::replacement)
  {
    exposition_only_advance_one();
    return *this;
  }

  constexpr auto operator++(int) {
    if constexpr (std::is_same_v<iterator_concept, std::input_iterator_tag>) {
      ++*this;
    } else {
      auto retval = *this;
      ++*this;
      return retval;
    }
  }

  constexpr exposition_only_iterator& operator--()
    requires std::ranges::bidirectional_range<exposition_only_Base>
  {
    if (!buf_index_)
      exposition_only_read_reverse();
    else
      --buf_index_;
    return *this;
  }

  constexpr exposition_only_iterator operator--(int)
    requires std::ranges::bidirectional_range<exposition_only_Base>
  {
    auto retval = *this;
    --*this;
    return retval;
  }

  friend constexpr bool operator==(const exposition_only_iterator& lhs,
                                   const exposition_only_iterator& rhs)
    requires std::equality_comparable<std::ranges::iterator_t<exposition_only_Base>>
  {
    return lhs.current_ == rhs.current_ && lhs.buf_index_ == rhs.buf_index_;
  }

private:
  /* !PAPER */

  struct decode_code_point_result {
    char32_t c;
    std::uint8_t to_incr;
    std::expected<void, utf_transcoding_error> success;
  };

  template <class>
  struct guard {
    constexpr guard(std::ranges::iterator_t<exposition_only_Base>&, std::ranges::iterator_t<exposition_only_Base>&) {
    }
  };

  template <class It>
    requires std::forward_iterator<It>
  struct guard<It> {
    constexpr ~guard() {
      curr = std::move(orig);
    }
    It& curr;
    It orig;
  };

  constexpr std::ranges::iterator_t<exposition_only_Base> begin() const
    requires std::ranges::bidirectional_range<exposition_only_Base>
  {
    return std::ranges::begin(parent_->base_);
  }

  /* PAPER */
  constexpr std::ranges::sentinel_t<exposition_only_Base> exposition_only_end() const { // @*exposition only*/
    return std::ranges::end(parent_->base_);
  }

  /* PAPER:       constexpr expected<void, utf_transcoding_error> @*success*@() const noexcept requires(E == to_utf_view_error_kind::expected); // @*exposition only*@ */
  /* !PAPER */

  constexpr bool exposition_only_success() const noexcept // @*exposition only*@
    requires(E == to_utf_view_error_kind::expected)
  {
    return !!success_;
  }

  /* PAPER */

  constexpr void exposition_only_advance_one() // @*exposition only*@
  {
    ++buf_index_;
    /* !PAPER */
    if (buf_index_ == static_cast<std::int8_t>(buf_.size())) {
    /* PAPER */
    /* PAPER:     if (buf_index_ == buf_.size()) { */
      if constexpr (std::ranges::forward_range<exposition_only_Base>) {
        buf_index_ = 0;
        std::advance(current_, to_increment_);
      }
      if (current_ != exposition_only_end()) {
        exposition_only_read();
      } else if constexpr (!std::ranges::forward_range<exposition_only_Base>) {
        buf_index_ = -1;
      }
    }
  }

  /* !PAPER */

  static constexpr decode_code_point_result decode_code_point_utf8_impl(
      std::ranges::iterator_t<exposition_only_Base>& it, std::ranges::sentinel_t<exposition_only_Base> const& last) {
    char32_t c{};
    std::uint8_t u = *it;
    ++it;
    const std::uint8_t lo_bound = 0x80, hi_bound = 0xBF;
    std::uint8_t to_incr = 1;
    std::expected<void, utf_transcoding_error> success{};

    auto const error{[&](utf_transcoding_error const error_enum_in) {
      success = std::unexpected{error_enum_in};
      c = U'\uFFFD';
    }};

    if (u <= 0x7F) [[likely]] // 0x00 to 0x7F
      c = u;
    else if (u < 0xC0) [[unlikely]] {
      error(utf_transcoding_error::unexpected_utf8_continuation_byte);
    } else if (u < 0xC2 || u > 0xF4) [[unlikely]] {
      error(utf_transcoding_error::invalid_utf8_leading_byte);
    } else if (it == last) [[unlikely]] {
      error(utf_transcoding_error::truncated_utf8_sequence);
    } else if (u <= 0xDF) // 0xC2 to 0xDF
    {
      c = u & 0x1F;
      u = *it;

      if (u < lo_bound || u > hi_bound) [[unlikely]]
        error(utf_transcoding_error::truncated_utf8_sequence);
      else {
        c = (c << 6) | (u & 0x3F);
        ++it;
        ++to_incr;
      }
    } else if (u <= 0xEF) // 0xE0 to 0xEF
    {
      std::uint8_t orig = u;
      c = u & 0x0F;
      u = *it;

      if (orig == 0xE0 && 0x80 <= u && u < 0xA0) [[unlikely]]
        error(utf_transcoding_error::overlong);
      else if (orig == 0xED && 0xA0 <= u && u < 0xC0) [[unlikely]]
        error(utf_transcoding_error::encoded_surrogate);
      else if (u < lo_bound || u > hi_bound) [[unlikely]]
        error(utf_transcoding_error::truncated_utf8_sequence);
      else if (++it == last) {
        [[unlikely]]++ to_incr;
        error(utf_transcoding_error::truncated_utf8_sequence);
      } else {
        ++to_incr;
        c = (c << 6) | (u & 0x3F);
        u = *it;

        if (u < lo_bound || u > hi_bound) [[unlikely]]
          error(utf_transcoding_error::truncated_utf8_sequence);
        else {
          c = (c << 6) | (u & 0x3F);
          ++it;
          ++to_incr;
        }
      }
    } else if (u <= 0xF4) // 0xF0 to 0xF4
    {
      std::uint8_t orig = u;
      c = u & 0x07;
      u = *it;

      if (orig == 0xF0 && 0x80 <= u && u < 0x90) [[unlikely]]
        error(utf_transcoding_error::overlong);
      else if (orig == 0xF4 && 0x90 <= u && u < 0xC0) [[unlikely]]
        error(utf_transcoding_error::out_of_range);
      else if (u < lo_bound || u > hi_bound) [[unlikely]]
        error(utf_transcoding_error::truncated_utf8_sequence);
      else if (++it == last) {
        [[unlikely]]++ to_incr;
        error(utf_transcoding_error::truncated_utf8_sequence);
      } else {
        ++to_incr;
        c = (c << 6) | (u & 0x3F);
        u = *it;

        if (u < lo_bound || u > hi_bound) [[unlikely]]
          error(utf_transcoding_error::truncated_utf8_sequence);
        else if (++it == last) {
          [[unlikely]]++ to_incr;
          error(utf_transcoding_error::truncated_utf8_sequence);
        } else {
          ++to_incr;
          c = (c << 6) | (u & 0x3F);
          u = *it;

          if (u < lo_bound || u > hi_bound) [[unlikely]]
            error(utf_transcoding_error::truncated_utf8_sequence);
          else {
            c = (c << 6) | (u & 0x3F);
            ++it;
            ++to_incr;
          }
        }
      }
    }

    return {.c{c}, .to_incr{to_incr}, .success{success}};
  }

  constexpr decode_code_point_result decode_code_point_utf8() {
    guard<std::ranges::iterator_t<exposition_only_Base>> g{current_, current_};
    return decode_code_point_utf8_impl(current_, exposition_only_end());
  }

  static constexpr decode_code_point_result decode_code_point_utf16_impl(
      std::ranges::iterator_t<exposition_only_Base>& it, std::ranges::sentinel_t<exposition_only_Base> const& last) {
    char32_t c{};
    std::uint16_t u = *it;
    ++it;
    std::uint8_t to_incr = 1;
    std::expected<void, utf_transcoding_error> success{};

    auto const error{[&](utf_transcoding_error const error_enum_in) {
      success = std::unexpected{error_enum_in};
      c = U'\uFFFD';
    }};

    if (u < 0xD800 || u > 0xDFFF) [[likely]]
      c = u;
    else if (u < 0xDC00) {
      if (it == last) [[unlikely]] {
        error(utf_transcoding_error::unpaired_high_surrogate);
      } else {
        std::uint16_t u2 = *it;
        if (u2 < 0xDC00 || u2 > 0xDFFF) [[unlikely]]
          error(utf_transcoding_error::unpaired_high_surrogate);
        else {
          ++it;
          to_incr = 2;
          std::uint32_t x = (u & 0x3F) << 10 | (u2 & 0x3FF);
          std::uint32_t w = (u >> 6) & 0x1F;
          c = (w + 1) << 16 | x;
        }
      }
    } else
      error(utf_transcoding_error::unpaired_low_surrogate);

    return {.c{c}, .to_incr{to_incr}, .success{success}};
  }

  constexpr decode_code_point_result decode_code_point_utf16() {
    guard<std::ranges::iterator_t<exposition_only_Base>> g{current_, current_};
    return decode_code_point_utf16_impl(current_, exposition_only_end());
  }

  static constexpr decode_code_point_result decode_code_point_utf32_impl(
      std::ranges::iterator_t<exposition_only_Base>& it) {
    char32_t c = *it;
    std::expected<void, utf_transcoding_error> success{};
    ++it;
    auto const error{[&](utf_transcoding_error const error_enum_in) {
      success = std::unexpected{error_enum_in};
      c = U'\uFFFD';
    }};
    if (c >= 0xD800) {
      if (c < 0xE000) {
        error(utf_transcoding_error::encoded_surrogate);
      }
      if (c > 0x10FFFF) {
        error(utf_transcoding_error::out_of_range);
      }
    }
    return {.c{c}, .to_incr{1}, .success{success}};
  }

  constexpr decode_code_point_result decode_code_point_utf32() {
    guard<std::ranges::iterator_t<exposition_only_Base>> g{current_, current_};
    return decode_code_point_utf32_impl(current_);
  }

  // Encode the code point c as one or more code units in buf.
  constexpr void update(char32_t c, std::uint8_t to_incr) {
    to_increment_ = to_incr;
    buf_index_ = 0;
    buf_.clear();
    if constexpr (std::is_same_v<ToType, char32_t>) {
      buf_.push_back(c);
    } else if constexpr (std::is_same_v<ToType, char16_t>) {
      if (c <= std::numeric_limits<char16_t>::max()) {
        buf_.push_back(static_cast<char16_t>(c));
      } else {
        // From http://www.unicode.org/faq/utf_bom.html#utf16-4
        const char32_t lead_offset = 0xD800 - (0x10000 >> 10);
        char16_t lead = lead_offset + (c >> 10);
        char16_t trail = 0xDC00 + (c & 0x3FF);
        buf_.push_back(lead);
        buf_.push_back(trail);
      }
    } else if constexpr (std::is_same_v<ToType, char8_t>) {
      int bits = std::bit_width(static_cast<std::uint32_t>(c));
      if (bits <= 7) [[likely]] {
        buf_.push_back(static_cast<char8_t>(c));
      } else if (bits <= 11) {
        buf_.push_back(0xC0 | (c >> 6));
        buf_.push_back(0x80 | (c & 0x3F));
      } else if (bits <= 16) {
        buf_.push_back(0xE0 | (c >> 12));
        buf_.push_back(0x80 | ((c >> 6) & 0x3F));
        buf_.push_back(0x80 | (c & 0x3F));
      } else {
        buf_.push_back(0xF0 | ((c >> 18) & 0x07));
        buf_.push_back(0x80 | ((c >> 12) & 0x3F));
        buf_.push_back(0x80 | ((c >> 6) & 0x3F));
        buf_.push_back(0x80 | (c & 0x3F));
      }
    } else {
      static_assert(false);
    }
  }

  /* PAPER:       constexpr void exposition_only_read(); // @*exposition only*@ */
  /* PAPER: */

  constexpr void exposition_only_read() { // @*exposition only*@
    success_.emplace();
    decode_code_point_result decode_result{};
    if constexpr (std::is_same_v<from_type, char8_t>)
      decode_result = decode_code_point_utf8();
    else if constexpr (std::is_same_v<from_type, char16_t>)
      decode_result = decode_code_point_utf16();
    else if constexpr (std::is_same_v<from_type, char32_t>) {
      decode_result = decode_code_point_utf32();
    } else {
      static_assert(false);
    }
    update(decode_result.c, decode_result.to_incr);
    success_ = decode_result.success;
  }

  struct read_reverse_impl_result {
    decode_code_point_result decode_result;
    std::ranges::iterator_t<exposition_only_Base> new_curr;
  };

  constexpr read_reverse_impl_result read_reverse_utf8() const {
    assert(current_ != begin());
    auto it{current_};
    auto const orig{it};
    unsigned reversed{};
    do {
      --it;
      ++reversed;
    } while (it != begin() && detail::continuation(*it) && reversed < 4);
    if (detail::continuation(*it)) {
      auto new_curr{orig};
      --new_curr;
      return {
          .decode_result{.c{U'\uFFFD'},
                         .to_incr{1},
                         .success{std::unexpected{
                             utf_transcoding_error::unexpected_utf8_continuation_byte}}},
          .new_curr{new_curr}};
    } else if (detail::is_ascii(*it) || detail::lead_code_unit(*it)) {
      int const expected_reversed{detail::utf8_code_units(*it)};
      assert(expected_reversed > 0);
      if (reversed > static_cast<unsigned>(expected_reversed)) {
        auto new_curr{orig};
        --new_curr;
        return {
            .decode_result{.c{U'\uFFFD'},
                           .to_incr{1},
                           .success{std::unexpected{
                               utf_transcoding_error::unexpected_utf8_continuation_byte}}},
            .new_curr{new_curr}};
      } else {
        auto lead{it};
        decode_code_point_result const decode_result{
            decode_code_point_utf8_impl(it, exposition_only_end())};
        if (decode_result.success ||
            decode_result.success ==
                std::unexpected{utf_transcoding_error::truncated_utf8_sequence}) {
          assert(decode_result.to_incr == reversed);
          return {.decode_result{decode_result}, .new_curr{lead}};
        } else {
          auto new_curr{orig};
          --new_curr;
          return {
              .decode_result{
                  .c{U'\uFFFD'},
                  .to_incr{1},
                  .success{
                      reversed == 1
                          ? decode_result.success
                          : std::unexpected{utf_transcoding_error::
                                                unexpected_utf8_continuation_byte}}},
              .new_curr{new_curr}};
        }
      }
    } else {
      assert(detail::in(0xC0, *it, 0xC2) || detail::in(0xF5, *it, 0xFF));
      it = orig;
      --it;
      return {
          .decode_result{
              .c{U'\uFFFD'},
              .to_incr{1},
              .success{
                  reversed == 1
                      ? std::unexpected{utf_transcoding_error::invalid_utf8_leading_byte}
                      : std::unexpected{utf_transcoding_error::
                                            unexpected_utf8_continuation_byte}}},
          .new_curr{it}};
    }
  }

  constexpr read_reverse_impl_result read_reverse_utf16() const {
    assert(current_ != begin());
    auto it{current_};
    auto const orig{it};
    --it;
    if (detail::high_surrogate(*it)) {
      return {.decode_result{.c{U'\uFFFD'},
                             .to_incr{1},
                             .success{std::unexpected{
                                 utf_transcoding_error::unpaired_high_surrogate}}},
              .new_curr{it}};
    } else if (detail::low_surrogate(*it)) {
      if (it == begin()) {
        return {.decode_result{.c{U'\uFFFD'},
                               .to_incr{1},
                               .success{std::unexpected{
                                   utf_transcoding_error::unpaired_low_surrogate}}},
                .new_curr{it}};
      } else {
        --it;
        if (detail::high_surrogate(*it)) {
          auto lead{it};
          return {.decode_result{decode_code_point_utf16_impl(it, exposition_only_end())},
                  .new_curr{lead}};
        } else {
          auto new_curr{orig};
          --new_curr;
          return {.decode_result{.c{U'\uFFFD'},
                                 .to_incr{1},
                                 .success{std::unexpected{
                                     utf_transcoding_error::unpaired_low_surrogate}}},
                  .new_curr{new_curr}};
        }
      }
    } else {
      return {.decode_result{.c{static_cast<char32_t>(*it)}, .to_incr{1}, .success{}},
              .new_curr{it}};
    }
  }

  constexpr read_reverse_impl_result read_reverse_utf32() const {
    assert(current_ != begin());
    auto it{current_};
    auto const orig{it};
    --it;
    auto new_curr{orig};
    --new_curr;
    return {.decode_result{decode_code_point_utf32_impl(it)}, .new_curr{new_curr}};
  }

  /* PAPER:       constexpr void exposition_only_read_reverse(); // @*exposition only*@ */

  constexpr void exposition_only_read_reverse() { // @*exposition only*@
    success_.emplace();
    auto const read_reverse_impl_result{[&] {
      if constexpr (std::is_same_v<from_type, char8_t>) {
        return read_reverse_utf8();
      } else if constexpr (std::is_same_v<from_type, char16_t>) {
        return read_reverse_utf16();
      } else if constexpr (std::is_same_v<from_type, char32_t>) {
        return read_reverse_utf32();
      }
    }()};
    update(read_reverse_impl_result.decode_result.c,
           read_reverse_impl_result.decode_result.to_incr);
    success_ = read_reverse_impl_result.decode_result.success;
    current_ = read_reverse_impl_result.new_curr;
    assert(buf_.size());
    buf_index_ = buf_.size() - 1;
    if constexpr (E == to_utf_view_error_kind::expected) {
      if (!success_.has_value()) {
        buf_index_ = 0;
      }
    }
  }

  /* PAPER */
};

template <std::ranges::input_range V, to_utf_view_error_kind E, exposition_only_code_unit ToType>
  requires std::ranges::view<V> && exposition_only_code_unit<std::ranges::range_value_t<V>>
template <bool Const>
struct to_utf_view<V, E, ToType>::exposition_only_sentinel {
private:
  using exposition_only_Parent = exposition_only_maybe_const<Const, to_utf_view>; // @*exposition only*@
  using exposition_only_Base = exposition_only_maybe_const<Const, V>; // @*exposition only*@
  std::ranges::sentinel_t<exposition_only_Base> end_ = std::ranges::sentinel_t<exposition_only_Base>();

public:
  exposition_only_sentinel() = default;
  constexpr explicit exposition_only_sentinel(std::ranges::sentinel_t<exposition_only_Base> end)
  : end_{end}
  {}
  constexpr explicit exposition_only_sentinel(exposition_only_sentinel<!Const> i)
    requires Const && std::convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<exposition_only_Base>>
  : end_{i.end_}
  {}

  constexpr std::ranges::sentinel_t<exposition_only_Base> base() const {
    return end_;
  }

  template<bool OtherConst>
    requires std::sentinel_for<std::ranges::sentinel_t<exposition_only_Base>, std::ranges::iterator_t<exposition_only_maybe_const<OtherConst, V>>>
  friend constexpr bool operator==(const exposition_only_iterator<OtherConst>& x, const exposition_only_sentinel& y) {
    if constexpr (std::ranges::forward_range<exposition_only_Base>) {
      return x.current_ == y.end_;
    } else {
      return x.current_ == y.end_ && x.buf_index_ == -1;
    }
  }
};

/* !PAPER */

namespace detail {

  template <to_utf_view_error_kind E, exposition_only_code_unit ToType>
  struct to_utf_impl : std::ranges::range_adaptor_closure<to_utf_impl<E, ToType>> {
    template <class R>
    constexpr auto operator()(R&& r) const {
      using T = std::remove_cvref_t<R>;
      if constexpr (detail::is_empty_view<T>) {
        return std::ranges::empty_view<ToType>{};
      } else if constexpr (std::is_bounded_array_v<T>) {
        constexpr auto n = std::extent_v<T>;
        auto first{std::ranges::begin(r)};
        auto last{std::ranges::end(r)};
        if (n && !r[n - 1]) {
          --last;
        }
        std::ranges::subrange subrange(first, last);
        return to_utf_view(std::move(subrange), detail::nontype<E>, to_utf_tag<ToType>);
      } else {
        return to_utf_view(std::forward<R>(r), detail::nontype<E>, to_utf_tag<ToType>);
      }
    }
  };

} // namespace detail

template <exposition_only_code_unit ToType>
inline constexpr detail::to_utf_impl<to_utf_view_error_kind::replacement, ToType> to_utf;

inline constexpr detail::to_utf_impl<to_utf_view_error_kind::replacement, char8_t> to_utf8;

inline constexpr detail::to_utf_impl<to_utf_view_error_kind::replacement, char16_t> to_utf16;

inline constexpr detail::to_utf_impl<to_utf_view_error_kind::replacement, char32_t> to_utf32;

template <exposition_only_code_unit ToType>
inline constexpr detail::to_utf_impl<to_utf_view_error_kind::expected, ToType> to_utf_or_error;

inline constexpr detail::to_utf_impl<to_utf_view_error_kind::expected, char8_t> to_utf8_or_error;

inline constexpr detail::to_utf_impl<to_utf_view_error_kind::expected, char16_t> to_utf16_or_error;

inline constexpr detail::to_utf_impl<to_utf_view_error_kind::expected, char32_t> to_utf32_or_error;

/* PAPER: namespace views {                                     */
/* PAPER:                                                       */
/* PAPER:   template<@*code-unit-to*@ ToType>                   */
/* PAPER:   inline constexpr @*unspecified*@ to_utf;            */
/* PAPER:                                                       */
/* PAPER:   template<@*code-unit-to*@ ToType>                   */
/* PAPER:   inline constexpr @*unspecified*@ to_utf_or_error;   */
/* PAPER:                                                       */
/* PAPER:   inline constexpr @*unspecified*@ to_utf8;           */
/* PAPER:                                                       */
/* PAPER:   inline constexpr @*unspecified*@ to_utf8_or_error;  */
/* PAPER:                                                       */
/* PAPER:   inline constexpr @*unspecified*@ to_utf16;          */
/* PAPER:                                                       */
/* PAPER:   inline constexpr @*unspecified*@ to_utf16_or_error; */
/* PAPER:                                                       */
/* PAPER:   inline constexpr @*unspecified*@ to_utf32;          */
/* PAPER:                                                       */
/* PAPER:   inline constexpr @*unspecified*@ to_utf32_or_error; */
/* PAPER:                                                       */
/* PAPER: }                                                     */
/* PAPER:                                                       */
/* PAPER: }                                                     */

} // namespace beman::utf_view

#endif // BEMAN_UTF_VIEW_TO_UTF_VIEW_HPP
