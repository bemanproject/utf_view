// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef BEMAN_UTF_VIEW_TO_UTF_VIEW_HPP
#define BEMAN_UTF_VIEW_TO_UTF_VIEW_HPP

#include <beman/utf_view/detail/concepts.hpp>
#include <beman/utf_view/detail/constexpr_unless_msvc.hpp>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <expected>
#include <iterator>
#include <limits>
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

} // namespace detail

/* PAPER: namespace std::ranges { */

/* PAPER */

template <class I>
consteval auto exposition_only_bidirectional_at_most() { // @*exposition only*@
  if constexpr (std::bidirectional_iterator<I>) {
    return std::bidirectional_iterator_tag{};
  } else if constexpr (std::forward_iterator<I>) {
    return std::forward_iterator_tag{};
  } else if constexpr (std::input_iterator<I>) {
    return std::input_iterator_tag{};
  }
}

template <class I>
using exposition_only_bidirectional_at_most_t =
    decltype(exposition_only_bidirectional_at_most<I>()); // @*exposition only*@

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

template <class V>
concept exposition_only_from_utf_view =
    exposition_only_utf_range<V> && std::ranges::view<V>;

template <bool OrError, exposition_only_code_unit ToType, exposition_only_from_utf_view V>
class exposition_only_to_utf_view_impl {
public:
  template <bool Const>
  class exposition_only_utf_iterator {
    /* !PAPER */
  public:
    using reserved_to_utf_view_iterator = void;

    /* PAPER */
  private:
    using exposition_only_iter =
        std::ranges::iterator_t<exposition_only_maybe_const<Const, V>>;
    using exposition_only_sent =
        std::ranges::sentinel_t<exposition_only_maybe_const<Const, V>>;

    template <bool OrError2, exposition_only_code_unit ToType2,
              exposition_only_from_utf_view V2>
    friend class exposition_only_to_utf_view_impl; // @*exposition only*@

    using exposition_only_from_type = std::iter_value_t<exposition_only_iter>; // @*exposition only*@

    using exposition_only_backptr_type =
        std::conditional_t<std::copyable<std::ranges::iterator_t<V>>,
                           exposition_only_to_utf_view_impl const*,
                           exposition_only_to_utf_view_impl*>; // @*exposition only*@

  public:
    using value_type =
        std::conditional_t<OrError, std::expected<ToType, utf_transcoding_error>, ToType>;
    using reference_type = value_type;
    using difference_type = ptrdiff_t;
    using iterator_concept =
        exposition_only_bidirectional_at_most_t<exposition_only_iter>;

    CONSTEXPR_UNLESS_MSVC exposition_only_utf_iterator()
      requires std::default_initializable<V>
    = default;

  private:
    constexpr exposition_only_utf_iterator(
        exposition_only_backptr_type parent, exposition_only_iter begin) // @*exposition only*@
        : backptr_(parent),
          base_(std::move(begin))
    {
      if (base() != end())
        read();
    }

    constexpr exposition_only_utf_iterator(exposition_only_to_utf_view_impl const* parent) // @*exposition only*@
        : backptr_(parent),
          base_(end())
    {
      if (base() != end())
        read();
    }

  public:
    CONSTEXPR_UNLESS_MSVC exposition_only_utf_iterator() = default;
    CONSTEXPR_UNLESS_MSVC exposition_only_utf_iterator(
        exposition_only_utf_iterator const&)
      requires std::copyable<exposition_only_iter>
    = default;

    CONSTEXPR_UNLESS_MSVC exposition_only_utf_iterator& operator=(
        exposition_only_utf_iterator const&)
      requires std::copyable<exposition_only_iter>
    = default;

    constexpr exposition_only_utf_iterator(exposition_only_utf_iterator&&) = default;

    constexpr exposition_only_utf_iterator& operator=(exposition_only_utf_iterator&&) =
        default;

    constexpr exposition_only_iter& base() & {
      return base_;
    }

    constexpr exposition_only_iter const& base() const& {
      return base_;
    }

    constexpr exposition_only_iter base() && {
      return std::move(base_);
    }

    /* PAPER:       constexpr expected<void, utf_transcoding_error> @*success*@() const noexcept requires(OrError); // @*exposition only*@ */
    /* !PAPER */

    constexpr bool exposition_only_success() const noexcept // @*exposition only*@
      requires(OrError)
    {
      return !!success_;
    }

    /* PAPER */

    /* PAPER:       constexpr value_type operator*() const; */
    /* !PAPER */
    constexpr value_type operator*() const {
      if constexpr (OrError) {
        if (!success_.has_value()) {
          return std::unexpected{success_.error()};
        }
      }
      return buf_[buf_index_];
    }
    /* PAPER */

    constexpr void exposition_only_advance_one() // @*exposition only*@
      requires std::forward_iterator<exposition_only_iter>
    {
      if (buf_index_ + 1 < buf_last_) {
        ++buf_index_;
      } else {
        std::advance(base(), to_increment_);
        to_increment_ = 0;
        if (base() != end()) {
          read();
        } else {
          buf_index_ = 0;
        }
      }
    }

    constexpr void exposition_only_advance_one() // @*exposition only*@
      requires (!std::forward_iterator<exposition_only_iter>)
    {
      if (buf_index_ + 1 == buf_last_ && base() != end()) {
        read();
      } else {
        ++buf_index_;
      }
    }

    constexpr exposition_only_utf_iterator& operator++() requires(OrError)
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

    constexpr exposition_only_utf_iterator& operator++() requires(!OrError)
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

    constexpr exposition_only_utf_iterator& operator--()
      requires std::bidirectional_iterator<exposition_only_iter>
    {
      if (!buf_index_)
        read_reverse();
      else if (buf_index_)
        --buf_index_;
      return *this;
    }

    constexpr exposition_only_utf_iterator operator--(int)
      requires std::bidirectional_iterator<exposition_only_iter>
    {
      auto retval = *this;
      --*this;
      return retval;
    }

    friend constexpr bool operator==(exposition_only_utf_iterator const& lhs,
                                     exposition_only_utf_iterator const& rhs)
      requires std::forward_iterator<exposition_only_iter> ||
        requires(exposition_only_iter i) { i != i; }
    {
      if constexpr (std::forward_iterator<exposition_only_iter>) {
        return lhs.base() == rhs.base() && lhs.buf_index_ == rhs.buf_index_;
      } else {
        if (lhs.base() != rhs.base())
          return false;

        if (lhs.buf_index_ == rhs.buf_index_ && lhs.buf_last_ == rhs.buf_last_) {
          return true;
        }

        return lhs.buf_index_ == lhs.buf_last_ && rhs.buf_index_ == rhs.buf_last_;
      }
    }

    friend constexpr bool operator==(exposition_only_utf_iterator const& lhs,
                                     exposition_only_sent rhs)
      requires std::copyable<exposition_only_iter>
    {
      if constexpr (std::forward_iterator<exposition_only_iter>) {
        return lhs.base() == rhs;
      } else {
        return lhs.base() == rhs && lhs.buf_index_ == lhs.buf_last_;
      }
    }

    friend constexpr bool operator==(exposition_only_utf_iterator const& lhs,
                                     exposition_only_sent rhs)
      requires(!std::copyable<exposition_only_iter>)
    {
      return lhs.base() == rhs && lhs.buf_index_ == lhs.buf_last_;
    }

    /* !PAPER */

  private:
    struct decode_code_point_result {
      char32_t c;
      std::uint8_t to_incr;
      std::expected<void, utf_transcoding_error> success;
    };

    template <class>
    struct guard {
      constexpr guard(exposition_only_iter&, exposition_only_iter&) {
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

    /* PAPER */

    constexpr exposition_only_iter begin() const // @*exposition only*@
      requires std::bidirectional_iterator<exposition_only_iter>
    {
      return std::ranges::begin(backptr_->base_);
    }
    constexpr exposition_only_sent end() const { // @*exposition only*@
      return std::ranges::end(backptr_->base_);
    }

    /* !PAPER */

    static constexpr decode_code_point_result decode_code_point_utf8_impl(
        exposition_only_iter& it, exposition_only_sent const& last) {
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
      guard<exposition_only_iter> g{base(), base()};
      return decode_code_point_utf8_impl(base(), end());
    }

    static constexpr decode_code_point_result decode_code_point_utf16_impl(
        exposition_only_iter& it, exposition_only_sent const& last) {
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
      guard<exposition_only_iter> g{base(), base()};
      return decode_code_point_utf16_impl(base(), end());
    }

    static constexpr decode_code_point_result decode_code_point_utf32_impl(
        exposition_only_iter& it) {
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
      guard<exposition_only_iter> g{base(), base()};
      return decode_code_point_utf32_impl(base());
    }

    // Encode the code point c as one or more code units in buf.
    constexpr void update(char32_t c, std::uint8_t to_incr) {
      to_increment_ = to_incr;
      buf_index_ = 0;
      if constexpr (std::is_same_v<ToType, char32_t>) {
        buf_[0] = c;
        buf_last_ = 1;
      } else if constexpr (std::is_same_v<ToType, char16_t>) {
        if (c <= std::numeric_limits<char16_t>::max()) {
          buf_[0] = static_cast<char16_t>(c);
          buf_[1] = 0;
          buf_last_ = 1;
        } else {
          // From http://www.unicode.org/faq/utf_bom.html#utf16-4
          const char32_t lead_offset = 0xD800 - (0x10000 >> 10);
          char16_t lead = lead_offset + (c >> 10);
          char16_t trail = 0xDC00 + (c & 0x3FF);
          buf_[0] = lead;
          buf_[1] = trail;
          buf_last_ = 2;
        }
      } else if constexpr (std::is_same_v<ToType, char8_t>) {
        int bits = std::bit_width(static_cast<std::uint32_t>(c));
        if (bits <= 7) [[likely]] {
          buf_[0] = static_cast<char8_t>(c);
          buf_[1] = buf_[2] = buf_[3] = 0;
          buf_last_ = 1;
        } else if (bits <= 11) {
          buf_[0] = 0xC0 | (c >> 6);
          buf_[1] = 0x80 | (c & 0x3F);
          buf_[2] = buf_[3] = 0;
          buf_last_ = 2;
        } else if (bits <= 16) {
          buf_[0] = 0xE0 | (c >> 12);
          buf_[1] = 0x80 | ((c >> 6) & 0x3F);
          buf_[2] = 0x80 | (c & 0x3F);
          buf_[3] = 0;
          buf_last_ = 3;
        } else {
          buf_[0] = 0xF0 | ((c >> 18) & 0x07);
          buf_[1] = 0x80 | ((c >> 12) & 0x3F);
          buf_[2] = 0x80 | ((c >> 6) & 0x3F);
          buf_[3] = 0x80 | (c & 0x3F);
          buf_last_ = 4;
        }
      } else {
        static_assert(false);
      }
    }

    /* PAPER:       constexpr void read(); // @*exposition only*@ */
    /* PAPER: */

    constexpr void read() { // @*exposition only*@
      success_.emplace();
      decode_code_point_result decode_result{};
      if constexpr (std::is_same_v<exposition_only_from_type, char8_t>)
        decode_result = decode_code_point_utf8();
      else if constexpr (std::is_same_v<exposition_only_from_type, char16_t>)
        decode_result = decode_code_point_utf16();
      else if constexpr (std::is_same_v<exposition_only_from_type, char32_t>) {
        decode_result = decode_code_point_utf32();
      } else {
        static_assert(false);
      }
      update(decode_result.c, decode_result.to_incr);
      success_ = decode_result.success;
    }

    struct read_reverse_impl_result {
      decode_code_point_result decode_result;
      exposition_only_iter new_curr;
    };

    constexpr read_reverse_impl_result read_reverse_utf8() const {
      assert(base() != begin());
      auto it{base()};
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
              decode_code_point_utf8_impl(it, end())};
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
      assert(base() != begin());
      auto it{base()};
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
            return {.decode_result{decode_code_point_utf16_impl(it, end())},
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
      assert(base() != begin());
      auto it{base()};
      auto const orig{it};
      --it;
      auto new_curr{orig};
      --new_curr;
      return {.decode_result{decode_code_point_utf32_impl(it)}, .new_curr{new_curr}};
    }

    /* PAPER:       constexpr void read_reverse(); // @*exposition only*@ */

    constexpr void read_reverse() { // @*exposition only*@
      success_.emplace();
      auto const read_reverse_impl_result{[&] {
        if constexpr (std::is_same_v<exposition_only_from_type, char8_t>) {
          return read_reverse_utf8();
        } else if constexpr (std::is_same_v<exposition_only_from_type, char16_t>) {
          return read_reverse_utf16();
        } else if constexpr (std::is_same_v<exposition_only_from_type, char32_t>) {
          return read_reverse_utf32();
        }
      }()};
      update(read_reverse_impl_result.decode_result.c,
             read_reverse_impl_result.decode_result.to_incr);
      success_ = read_reverse_impl_result.decode_result.success;
      base() = read_reverse_impl_result.new_curr;
      assert(buf_last_);
      buf_index_ = buf_last_ - 1;
      if constexpr (OrError) {
        if (!success_.has_value()) {
          buf_index_ = 0;
        }
      }
    }

    /* PAPER */
    std::array<value_type, 4 / sizeof(ToType)> buf_{}; // @*exposition only*@

    exposition_only_backptr_type backptr_;

    exposition_only_iter base_;

    std::uint8_t buf_index_ = 0; // @*exposition only*@
    std::uint8_t buf_last_ = 0; // @*exposition only*@
    std::uint8_t to_increment_ = 0; // @*exposition only*@

    /* !PAPER */
    std::expected<void, utf_transcoding_error> success_{}; // @*exposition only*@
    /* PAPER */
  };

private:
  V base_ = V(); // @*exposition only*@

  constexpr auto make_begin(this auto& self) { // @*exposition only*@
    constexpr bool const_{std::is_const_v<std::remove_reference_t<decltype(self)>>};
    return exposition_only_utf_iterator<const_>(&self, std::ranges::begin(self.base_));
  }
  constexpr auto make_end(this auto& self) { // @*exposition only*@
    if constexpr (std::bidirectional_iterator<std::ranges::sentinel_t<V>>) {
      constexpr bool const_{std::is_const_v<std::remove_reference_t<decltype(self)>>};
      return exposition_only_utf_iterator<const_>(&self);
    } else {
      return std::ranges::end(self.base_);
    }
  }

public:
  constexpr exposition_only_to_utf_view_impl()
    requires std::default_initializable<V>
  = default;
  constexpr exposition_only_to_utf_view_impl(V base)
      : base_(std::move(base)) { }

  constexpr V base() const&
    requires std::copy_constructible<V>
  {
    return base_;
  }
  constexpr V base() && {
    return std::move(base_);
  }

  constexpr auto begin()
    requires(!std::copyable<std::ranges::iterator_t<V>>)
  {
    return make_begin();
  }
  constexpr auto begin() const
    requires std::copyable<std::ranges::iterator_t<V>>
  {
    return make_begin();
  }

  constexpr auto end()
    requires(!std::copyable<std::ranges::iterator_t<V>>)
  {
    return make_end();
  }
  constexpr auto end() const
    requires std::copyable<std::ranges::iterator_t<V>>
  {
    return make_end();
  }

  constexpr bool empty() const {
    return std::ranges::empty(base_);
  }
};

template <exposition_only_from_utf_view V>
class to_utf8_view : public std::ranges::view_interface<to_utf8_view<V>> {
private:
  using exposition_only_iterator =
      std::ranges::iterator_t<exposition_only_to_utf_view_impl<false, char8_t, V>>;
  using exposition_only_sentinel =
      std::ranges::sentinel_t<exposition_only_to_utf_view_impl<false, char8_t, V>>;

public:
  constexpr to_utf8_view()
    requires std::default_initializable<V>
  = default;
  constexpr to_utf8_view(V base)
      : impl_(std::move(base)) { }

  constexpr V base() const&
    requires std::copy_constructible<V>
  {
    return impl_.base();
  }
  constexpr V base() && {
    return std::move(impl_).base();
  }

  constexpr auto begin()
    requires(!std::copyable<exposition_only_iterator>)
  {
    return impl_.begin();
  }
  constexpr auto begin() const
    requires std::copyable<exposition_only_iterator>
  {
    return impl_.begin();
  }

  constexpr auto end()
    requires(!std::copyable<exposition_only_iterator>)
  {
    return impl_.end();
  }
  constexpr auto end() const
    requires std::copyable<exposition_only_iterator>
  {
    return impl_.end();
  }

  constexpr bool empty() const {
    return impl_.empty();
  }

private:
  exposition_only_to_utf_view_impl<false, char8_t, V> impl_;
};

template <class R>
to_utf8_view(R&&) -> to_utf8_view<std::views::all_t<R>>;

template <exposition_only_from_utf_view V>
class to_utf8_or_error_view
    : public std::ranges::view_interface<to_utf8_or_error_view<V>> {
private:
  using exposition_only_iterator =
      std::ranges::iterator_t<exposition_only_to_utf_view_impl<true, char8_t, V>>;
  using exposition_only_sentinel =
      std::ranges::sentinel_t<exposition_only_to_utf_view_impl<true, char8_t, V>>;

public:
  constexpr to_utf8_or_error_view()
    requires std::default_initializable<V>
  = default;
  constexpr to_utf8_or_error_view(V base)
      : impl_(std::move(base)) { }

  constexpr V base() const&
    requires std::copy_constructible<V>
  {
    return impl_.base();
  }
  constexpr V base() && {
    return std::move(impl_).base();
  }

  constexpr auto begin()
    requires(!std::copyable<exposition_only_iterator>)
  {
    return impl_.begin();
  }
  constexpr auto begin() const
    requires std::copyable<exposition_only_iterator>
  {
    return impl_.begin();
  }

  constexpr auto end()
    requires(!std::copyable<exposition_only_iterator>)
  {
    return impl_.end();
  }
  constexpr auto end() const
    requires std::copyable<exposition_only_iterator>
  {
    return impl_.end();
  }

  constexpr bool empty() const {
    return impl_.empty();
  }

private:
  exposition_only_to_utf_view_impl<true, char8_t, V> impl_;
};

template <class R>
to_utf8_or_error_view(R&&) -> to_utf8_or_error_view<std::views::all_t<R>>;

template <exposition_only_from_utf_view V>
class to_utf16_view : public std::ranges::view_interface<to_utf16_view<V>> {
private:
  using exposition_only_iterator =
      std::ranges::iterator_t<exposition_only_to_utf_view_impl<false, char16_t, V>>;
  using exposition_only_sentinel =
      std::ranges::sentinel_t<exposition_only_to_utf_view_impl<false, char16_t, V>>;

public:
  constexpr to_utf16_view()
    requires std::default_initializable<V>
  = default;
  constexpr to_utf16_view(V base)
      : impl_(std::move(base)) { }

  constexpr V base() const&
    requires std::copy_constructible<V>
  {
    return impl_.base();
  }
  constexpr V base() && {
    return std::move(impl_).base();
  }

  constexpr auto begin()
    requires(!std::copyable<exposition_only_iterator>)
  {
    return impl_.begin();
  }
  constexpr auto begin() const
    requires std::copyable<exposition_only_iterator>
  {
    return impl_.begin();
  }

  constexpr auto end()
    requires(!std::copyable<exposition_only_iterator>)
  {
    return impl_.end();
  }
  constexpr auto end() const
    requires std::copyable<exposition_only_iterator>
  {
    return impl_.end();
  }

  constexpr bool empty() const {
    return impl_.empty();
  }

private:
  exposition_only_to_utf_view_impl<false, char16_t, V> impl_;
};

template <class R>
to_utf16_view(R&&) -> to_utf16_view<std::views::all_t<R>>;

template <exposition_only_from_utf_view V>
class to_utf16_or_error_view
    : public std::ranges::view_interface<to_utf16_or_error_view<V>> {
private:
  using exposition_only_iterator =
      std::ranges::iterator_t<exposition_only_to_utf_view_impl<true, char16_t, V>>;
  using exposition_only_sentinel =
      std::ranges::sentinel_t<exposition_only_to_utf_view_impl<true, char16_t, V>>;

public:
  constexpr to_utf16_or_error_view()
    requires std::default_initializable<V>
  = default;
  constexpr to_utf16_or_error_view(V base)
      : impl_(std::move(base)) { }

  constexpr V base() const&
    requires std::copy_constructible<V>
  {
    return impl_.base();
  }
  constexpr V base() && {
    return std::move(impl_).base();
  }

  constexpr auto begin()
    requires(!std::copyable<exposition_only_iterator>)
  {
    return impl_.begin();
  }
  constexpr auto begin() const
    requires std::copyable<exposition_only_iterator>
  {
    return impl_.begin();
  }

  constexpr auto end()
    requires(!std::copyable<exposition_only_iterator>)
  {
    return impl_.end();
  }
  constexpr auto end() const
    requires std::copyable<exposition_only_iterator>
  {
    return impl_.end();
  }

  constexpr bool empty() const {
    return impl_.empty();
  }

private:
  exposition_only_to_utf_view_impl<true, char16_t, V> impl_;
};

template <class R>
to_utf16_or_error_view(R&&) -> to_utf16_or_error_view<std::views::all_t<R>>;

template <exposition_only_from_utf_view V>
class to_utf32_view : public std::ranges::view_interface<to_utf32_view<V>> {
private:
  using exposition_only_iterator =
      std::ranges::iterator_t<exposition_only_to_utf_view_impl<false, char32_t, V>>;
  using exposition_only_sentinel =
      std::ranges::sentinel_t<exposition_only_to_utf_view_impl<false, char32_t, V>>;

public:
  constexpr to_utf32_view()
    requires std::default_initializable<V>
  = default;
  constexpr to_utf32_view(V base)
      : impl_(std::move(base)) { }

  constexpr V base() const&
    requires std::copy_constructible<V>
  {
    return impl_.base();
  }
  constexpr V base() && {
    return std::move(impl_).base();
  }

  constexpr auto begin()
    requires(!std::copyable<exposition_only_iterator>)
  {
    return impl_.begin();
  }
  constexpr auto begin() const
    requires std::copyable<exposition_only_iterator>
  {
    return impl_.begin();
  }

  constexpr auto end()
    requires(!std::copyable<exposition_only_iterator>)
  {
    return impl_.end();
  }
  constexpr auto end() const
    requires std::copyable<exposition_only_iterator>
  {
    return impl_.end();
  }

  constexpr bool empty() const {
    return impl_.empty();
  }

private:
  exposition_only_to_utf_view_impl<false, char32_t, V> impl_;
};

template <class R>
to_utf32_view(R&&) -> to_utf32_view<std::views::all_t<R>>;

template <exposition_only_from_utf_view V>
class to_utf32_or_error_view
    : public std::ranges::view_interface<to_utf32_or_error_view<V>> {
private:
  using exposition_only_iterator =
      std::ranges::iterator_t<exposition_only_to_utf_view_impl<true, char32_t, V>>;
  using exposition_only_sentinel =
      std::ranges::sentinel_t<exposition_only_to_utf_view_impl<true, char32_t, V>>;

public:
  constexpr to_utf32_or_error_view()
    requires std::default_initializable<V>
  = default;
  constexpr to_utf32_or_error_view(V base)
      : impl_(std::move(base)) { }

  constexpr V base() const&
    requires std::copy_constructible<V>
  {
    return impl_.base();
  }
  constexpr V base() && {
    return std::move(impl_).base();
  }

  constexpr auto begin()
  {
    return impl_.begin();
  }
  constexpr auto begin() const
  {
    return impl_.begin();
  }

  constexpr auto end()
  {
    return impl_.end();
  }
  constexpr auto end() const
  {
    return impl_.end();
  }

  constexpr bool empty() const {
    return impl_.empty();
  }

private:
  exposition_only_to_utf_view_impl<true, char32_t, V> impl_;
};

template <class R>
to_utf32_or_error_view(R&&) -> to_utf32_or_error_view<std::views::all_t<R>>;

/* !PAPER */

namespace detail {

  template <bool OrError, exposition_only_code_unit ToType,
            exposition_only_from_utf_view V>
  using to_utf_view = std::conditional_t<
      OrError,
      std::conditional_t<
          std::is_same_v<ToType, char8_t>, to_utf8_or_error_view<V>,
          std::conditional_t<std::is_same_v<ToType, char16_t>, to_utf16_or_error_view<V>,
                             std::conditional_t<std::is_same_v<ToType, char32_t>,
                                                to_utf32_or_error_view<V>, void>>>,
      std::conditional_t<
          std::is_same_v<ToType, char8_t>, to_utf8_view<V>,
          std::conditional_t<std::is_same_v<ToType, char16_t>, to_utf16_view<V>,
                             std::conditional_t<std::is_same_v<ToType, char32_t>,
                                                to_utf32_view<V>, void>>>>;

  template <bool OrError, exposition_only_code_unit ToType>
  struct to_utf_impl : std::ranges::range_adaptor_closure<to_utf_impl<OrError, ToType>> {
    template <exposition_only_utf_range R>
    constexpr auto operator()(R&& r) const {
      using T = std::remove_cvref_t<R>;
      if constexpr (exposition_only_is_empty_view<T>) {
        return std::ranges::empty_view<ToType>{};
      } else if constexpr (std::is_bounded_array_v<T>) {
        constexpr auto n = std::extent_v<T>;
        auto first{std::ranges::begin(r)};
        auto last{std::ranges::end(r)};
        if (n && !r[n - 1]) {
          --last;
        }
        std::ranges::subrange subrange(first, last);
        return to_utf_view<OrError, ToType, decltype(subrange)>(std::move(subrange));
      } else {
        auto view = std::views::all(std::forward<R>(r));
        return to_utf_view<OrError, ToType, decltype(view)>(std::move(view));
      }
    }
  };

} // namespace detail

template <exposition_only_code_unit ToType>
inline constexpr detail::to_utf_impl<false, ToType> to_utf;

inline constexpr detail::to_utf_impl<false, char8_t> to_utf8;

inline constexpr detail::to_utf_impl<false, char16_t> to_utf16;

inline constexpr detail::to_utf_impl<false, char32_t> to_utf32;

template <exposition_only_code_unit ToType>
inline constexpr detail::to_utf_impl<true, ToType> to_utf_or_error;

inline constexpr detail::to_utf_impl<true, char8_t> to_utf8_or_error;

inline constexpr detail::to_utf_impl<true, char16_t> to_utf16_or_error;

inline constexpr detail::to_utf_impl<true, char32_t> to_utf32_or_error;

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
