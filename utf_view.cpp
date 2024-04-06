module;

#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <iterator>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>

export module p2728:utf_view;
import :concepts;

namespace p2728 {

  using namespace std;

  constexpr char16_t high_surrogate_base = 0xd7c0;
  constexpr char16_t low_surrogate_base = 0xdc00;
  constexpr char32_t high_surrogate_min = 0xd800;
  constexpr char32_t high_surrogate_max = 0xdbff;
  constexpr char32_t low_surrogate_min = 0xdc00;
  constexpr char32_t low_surrogate_max = 0xdfff;
  constexpr char32_t replacement_character = 0xfffd;

  constexpr bool in(unsigned char lo, unsigned char c, unsigned char hi) { return lo <= c && c <= hi; }

  constexpr bool surrogate(char32_t c) { return high_surrogate_min <= c && c <= low_surrogate_max; }

  constexpr bool high_surrogate(char32_t c) { return high_surrogate_min <= c && c <= high_surrogate_max; }

  constexpr bool lead_code_unit(unsigned char c) { return uint8_t(c - 0xc2) <= 0x32; }

  constexpr bool continuation(unsigned char c) { return (int8_t)c < -0x40; }

  constexpr int utf8_code_units(unsigned char first_unit) {
    return first_unit <= 0x7f ? 1 : lead_code_unit(first_unit) ? int(0xe0 <= first_unit) + int(0xf0 <= first_unit) + 2 : -1;
  }

  export enum class transcoding_error {
    truncated, // e.g. utf8 0xE1 0x80 utf16 0xD800
    unexpected_continuation, // e.g. utf8 0x80 utf16 0xDC00
    overlong, // e.g. utf8 0xE0 0x80
    surrogate, // e.g. utf8 0xED 0xA0, utf32 0x0000D800
    out_of_range, // e.g. utf8 0xF4 0x90, utf32 0x00110000
  };

  export template<EOcode_unitOE ToType, EOutf_rangeOE V>
  requires ranges::view<V>
  class utf_view : public ranges::view_interface<utf_view<ToType, V>> {
  private:
    using EOiterOE = ranges::iterator_t<V>;
    using EOsentOE = ranges::sentinel_t<V>;

  public:
    class utf_iterator {
    private:
      template<class I>
      struct EOfirst_and_currOE {                         // @*exposition only*@
        EOfirst_and_currOE() = default;
        constexpr EOfirst_and_currOE(I curr) : curr{move(curr)} { }
        template<class I2>
        requires convertible_to<I2, I>
        constexpr EOfirst_and_currOE(const EOfirst_and_currOE<I2>& other) : curr{other.curr} { }

        I curr;
      };
      template<bidirectional_iterator I>
      struct EOfirst_and_currOE<I> {                // @*exposition only*@
        EOfirst_and_currOE() = default;
        constexpr EOfirst_and_currOE(I first, I curr) : first{first}, curr{curr} { }
        template<class I2>
        requires convertible_to<I2, I>
        constexpr EOfirst_and_currOE(const EOfirst_and_currOE<I2>& other) : first{other.first}, curr{other.curr} { }

        I first;
        I curr;
      };

      template<class I>
      static constexpr auto EObidirectional_at_mostOE() {    // @*exposition only*@
        if constexpr (bidirectional_iterator<I>) {
          return bidirectional_iterator_tag{};
        } else if constexpr (forward_iterator<I>) {
          return forward_iterator_tag{};
        } else if constexpr (input_iterator<I>) {
          return input_iterator_tag{};
        }
      }

      template<class I>
      using EObidirectional_at_most_tOE = decltype(EObidirectional_at_mostOE<I>()); // @*exposition only*@

    public:
      using value_type = ToType;
      using reference_type = ToType&;
      using difference_type = ptrdiff_t;
      using iterator_concept = EObidirectional_at_most_tOE<EOiterOE>;

// https://developercommunity.visualstudio.com/t/MSVC-complains-about-uninvoked-implicitl/10585513
#ifndef _MSC_VER
      constexpr utf_iterator() requires default_initializable<V>
      = default;
#endif

      constexpr utf_iterator(EOiterOE first, EOiterOE it, EOsentOE last) requires bidirectional_iterator<EOiterOE>
          : first_and_curr_{first, it}, last_(last) {
        if (curr() != last_)
          read();
      }
      constexpr utf_iterator(EOiterOE /* first */, EOiterOE it, EOsentOE last) requires (!bidirectional_iterator<EOiterOE>)
          : first_and_curr_{move(it)}, last_(last) {
        if (curr() != last_)
          read();
      }
      constexpr utf_iterator(EOiterOE it, EOsentOE last) requires (!bidirectional_iterator<EOiterOE>)
          : first_and_curr_{move(it)}, last_(last) {
        if (curr() != last_)
          read();
      }

      template<class V2>
      requires convertible_to<ranges::iterator_t<V2>, EOiterOE> && convertible_to<ranges::sentinel_t<V2>, EOsentOE> &&
                 copyable<V2>
      constexpr utf_iterator(const typename utf_view<ToType, V2>::utf_iterator& other) :
          buf_(other.buf_),
          first_and_curr_(other.first_and_curr_),
          buf_index_(other.buf_index_),
          buf_last_(other.buf_last_),
          last_(other.last_) { }

      template<class V2>
      requires convertible_to<ranges::iterator_t<V2>, EOiterOE> && convertible_to<ranges::sentinel_t<V2>, EOsentOE> &&
        copyable<V2>
      constexpr utf_iterator& operator=(const typename utf_view<ToType, V2>::utf_iterator& other) {
        buf_ = other.buf_;
        first_and_curr_ = other.first_and_curr_;
        buf_index_ = other.buf_index_;
        buf_last_ = other.buf_last_;
        last_ = other.last_;
      }

      template<class V2>
      requires convertible_to<ranges::iterator_t<V2>, EOiterOE> && convertible_to<ranges::sentinel_t<V2>, EOsentOE> && movable<V2>
      constexpr utf_iterator(typename utf_view<ToType, V2>::utf_iterator&& other) :
          buf_(other.buf_),
          first_and_curr_(move(other.first_and_curr_)),
          buf_index_(other.buf_index_),
          buf_last_(other.buf_last_),
          last_(other.last_) { }

      template<class V2>
      requires convertible_to<ranges::iterator_t<V2>, EOiterOE> && convertible_to<ranges::sentinel_t<V2>, EOsentOE> && movable<V2>
      constexpr utf_iterator& operator=(const typename utf_view<ToType, V2>::utf_iterator& other) {
        buf_ = other.buf_;
        first_and_curr_ = move(other.first_and_curr_);
        buf_index_ = other.buf_index_;
        buf_last_ = other.buf_last_;
        last_ = other.last_;
      }

      constexpr EOiterOE begin() const requires bidirectional_iterator<EOiterOE>
      {
        return first();
      }
      constexpr EOsentOE end() const { return last_; }

      constexpr EOiterOE base() const requires forward_iterator<EOiterOE>
      {
        return curr();
      }

      constexpr optional<transcoding_error> error() const { return error_; }

      constexpr value_type operator*() const { return buf_[buf_index_]; }

      constexpr value_type* operator->() const { return &buf_[buf_index_]; }

      constexpr utf_iterator& operator++() {
        if (buf_index_ + 1 == buf_last_ && curr() != last_) {
          if constexpr (forward_iterator<EOiterOE>) {
            advance(curr(), to_increment_);
          }
          if (curr() == last_)
            buf_index_ = 0;
          else
            read();
        } else if (buf_index_ + 1 <= buf_last_) {
          ++buf_index_;
        }
        return *this;
      }

      constexpr auto operator++(int) {
        if constexpr (is_same_v<iterator_concept, input_iterator_tag>) {
          ++*this;
        } else {
          auto retval = *this;
          ++*this;
          return retval;
        }
      }

      constexpr utf_iterator& operator--() requires bidirectional_iterator<EOiterOE>
      {
        if (!buf_index_ && curr() != first())
          read_reverse();
        else if (buf_index_)
          --buf_index_;
        return *this;
      }

      constexpr utf_iterator operator--(int) requires bidirectional_iterator<EOiterOE>
      {
        auto retval = *this;
        --*this;
        return retval;
      }

      friend constexpr bool operator==(utf_iterator lhs, utf_iterator rhs)
        requires forward_iterator<EOiterOE> || requires (EOiterOE i) { i != i; }
      {
        if constexpr (forward_iterator<EOiterOE>) {
          return lhs.curr() == rhs.curr() && lhs.buf_index_ == rhs.buf_index_;
        } else {
          if (lhs.curr() != rhs.curr())
            return false;

          if (lhs.buf_index_ == rhs.buf_index_ && lhs.buf_last_ == rhs.buf_last_) {
            return true;
          }

          return lhs.buf_index_ == lhs.buf_last_ && rhs.buf_index_ == rhs.buf_last_;
        }
      }

      friend constexpr bool operator==(utf_iterator lhs, EOsentOE rhs) requires copyable<EOiterOE>
      {
        if constexpr (forward_iterator<EOiterOE>) {
          return lhs.curr() == rhs;
        } else {
          return lhs.curr() == rhs && lhs.buf_index_ == lhs.buf_last_;
        }
      }

      friend constexpr bool operator==(utf_iterator const& lhs, EOsentOE rhs) requires (!copyable<EOiterOE>)
      {
        return lhs.curr() == rhs && lhs.buf_index_ == lhs.buf_last_;
      }

    private:
      struct decode_code_point_result {
        char32_t c;
        uint8_t to_incr;
      };

      template<typename>
      struct guard {
        constexpr guard(void*, EOiterOE&) { }
      };

      template<typename It>
      requires forward_iterator<It>
      struct guard<It> {
        constexpr ~guard() { iter->curr() = std::move(orig); }
        utf_iterator* iter;
        It orig;
      };

      // A code point that can be encoded in a single code unit of type CharT.
      template<class CharT>
      constexpr bool is_single_code_unit(char32_t c) {
        if constexpr (numeric_limits<CharT>::max() <= 0xFF)
          return c < 0x7F; // ASCII character
        else
          return c < numeric_limits<CharT>::max();
      }

      constexpr decode_code_point_result decode_code_point_utf8() {
        guard<EOiterOE> g{this, curr()};
        char32_t c{};
        uint8_t u = *curr();
        ++curr();
        const uint8_t lo_bound = 0x80, hi_bound = 0xBF;
        uint8_t to_incr = 1;
        error_.reset();

        auto const error{[&](transcoding_error const error) {
          error_ = error;
          c = U'\uFFFD';
        }};

        if (u <= 0x7F) [[likely]]      // 0x00 to 0x7F
          c = u;
        else if (u < 0xC0) [[unlikely]] {
          error(transcoding_error::unexpected_continuation);
        } else if (u < 0xC2) [[unlikely]] {
          error(transcoding_error::overlong);
        } else if (curr() == last_) [[unlikely]] {
          error(transcoding_error::truncated);
        } else if (u <= 0xDF) // 0xC2 to 0xDF
        {
          c = u & 0x1F;
          u = *curr();

          if (u < lo_bound || u > hi_bound) [[unlikely]]
            error(transcoding_error::truncated);
          else {
            c = (c << 6) | (u & 0x3F);
            ++curr();
            ++to_incr;
          }
        } else if (u <= 0xEF) // 0xE0 to 0xEF
        {
          uint8_t orig = u;
          c = u & 0x0F;
          u = *curr();

          if (orig == 0xE0 && 0x80 <= u && u < 0xA0) [[unlikely]]
            error(transcoding_error::overlong);
          else if (orig == 0xED && 0xA0 <= u && u < 0xC0) [[unlikely]]
            error(transcoding_error::surrogate);
          else if (u < lo_bound || u > hi_bound) [[unlikely]]
            error(transcoding_error::truncated);
          else if (++curr() == last_) {
            [[unlikely]]++ to_incr;
            error(transcoding_error::truncated);
          } else {
            ++to_incr;
            c = (c << 6) | (u & 0x3F);
            u = *curr();

            if (u < lo_bound || u > hi_bound) [[unlikely]]
              error(transcoding_error::truncated);
            else {
              c = (c << 6) | (u & 0x3F);
              ++curr();
              ++to_incr;
            }
          }
        } else if (u <= 0xF4) // 0xF0 to 0xF4
        {
          uint8_t orig = u;
          c = u & 0x07;
          u = *curr();

          if (orig == 0xF0 && 0x80 <= u && u < 0x90) [[unlikely]]
            error(transcoding_error::overlong);
          else if (orig == 0xF4 && 0x90 <= u && u < 0xC0) [[unlikely]]
            error(transcoding_error::out_of_range);
          else if (u < lo_bound || u > hi_bound) [[unlikely]]
            error(transcoding_error::truncated);
          else if (++curr() == last_) {
            [[unlikely]]++ to_incr;
            error(transcoding_error::truncated);
          } else {
            ++to_incr;
            c = (c << 6) | (u & 0x3F);
            u = *curr();

            if (u < lo_bound || u > hi_bound) [[unlikely]]
              error(transcoding_error::truncated);
            else if (++curr() == last_) {
              [[unlikely]]++ to_incr;
              error(transcoding_error::truncated);
            } else {
              ++to_incr;
              c = (c << 6) | (u & 0x3F);
              u = *curr();

              if (u < lo_bound || u > hi_bound) [[unlikely]]
                error(transcoding_error::truncated);
              else {
                c = (c << 6) | (u & 0x3F);
                ++curr();
                ++to_incr;
              }
            }
          }
        } else [[unlikely]]
          error(transcoding_error::out_of_range);

        return {.c{c}, .to_incr{to_incr}};
      }

      constexpr decode_code_point_result decode_code_point_utf16() {
        guard<EOiterOE> g{this, curr()};
        char32_t c{};
        uint16_t u = *curr();
        ++curr();
        uint8_t to_incr = 1;

        auto const error{[&](transcoding_error const error) {
          error_ = error;
          c = U'\uFFFD';
        }};

        if (u < 0xD800 || u > 0xDFFF) [[likely]]
          c = u;
        else if (u < 0xDC00) {
          if (curr() == last_) [[unlikely]] {
            error(transcoding_error::truncated);
          } else {
            uint16_t u2 = *curr();
            if (u2 < 0xDC00 || u2 > 0xDFFF) [[unlikely]]
              error(transcoding_error::truncated);
            else {
              ++curr();
              to_incr = 2;
              uint32_t x = (u & 0x3F) << 10 | u2 & 0x3FF;
              uint32_t w = (u >> 6) & 0x1F;
              c = (w + 1) << 16 | x;
            }
          }
        } else
          error(transcoding_error::unexpected_continuation);

        return {.c{c}, .to_incr{to_incr}};
      }

      constexpr decode_code_point_result decode_code_point_utf32() {
        guard<EOiterOE> g{this, curr()};
        char32_t c = *curr();
        ++curr();
        auto const error{[&](transcoding_error const error) {
          error_ = error;
          c = U'\uFFFD';
        }};
        if (c >= 0xD800) {
          if (c < 0xE000) {
            error(transcoding_error::surrogate);
          }
          if (c > 0x10FFFF) {
            error(transcoding_error::out_of_range);
          }
        }
        return {.c{c}, .to_incr{1}};
      }

      // Encode the code point c as one or more code units in buf.
      constexpr void update(char32_t c, uint8_t to_incr) {
        to_increment_ = to_incr;
        buf_index_ = 0;
        if constexpr (is_same_v<ToType, char32_t>) {
          buf_[0] = c;
          buf_last_ = 1;
        } else if constexpr (is_same_v<ToType, char16_t>) {
          if (is_single_code_unit<ToType>(c)) {
            buf_[0] = c;
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
        } else if constexpr (is_same_v<ToType, char8_t>) {
          int bits = bit_width((uint32_t)c);
          if (bits <= 7) [[likely]] {
            buf_[0] = c;
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
          unreachable(); // todo: static_assert(false) once CWG2518 is available
        }
      }

      constexpr void read() {                                            // @*exposition only*@
        error_.reset();
        decode_code_point_result decode_result{};
        // todo: support char
        if constexpr (is_same_v<iter_value_t<EOiterOE>, char8_t>)
          decode_result = decode_code_point_utf8();
        // todo: support wchar_t
        else if constexpr (is_same_v<iter_value_t<EOiterOE>, char16_t>)
          decode_result = decode_code_point_utf16();
        else {
          static_assert(is_same_v<iter_value_t<EOiterOE>, char32_t>);
          decode_result = decode_code_point_utf32();
        }
        update(decode_result.c, decode_result.to_incr);
      }
      constexpr void read_reverse() { // @*exposition only*@
        throw std::runtime_error{"unimpl"};
      }

      constexpr EOiterOE first() const requires bidirectional_iterator<EOiterOE>      // @*exposition only*@
      {
        return first_and_curr_.first;
      }
      constexpr EOiterOE& curr() { return first_and_curr_.curr; }              // @*exposition only*@
      constexpr EOiterOE const& curr() const { return first_and_curr_.curr; }         // @*exposition only*@

      array<value_type, 4 / sizeof(ToType)> buf_;           // @*exposition only*@

      EOfirst_and_currOE<EOiterOE> first_and_curr_;                                // @*exposition only*@

      uint8_t buf_index_ = 0;                                           // @*exposition only*@
      uint8_t buf_last_ = 0;                                            // @*exposition only*@
      uint8_t to_increment_ = 0;                                        // @*exposition only*@

      [[no_unique_address]] EOsentOE last_;                                    // @*exposition only*@

      optional<transcoding_error> error_;                             // @*exposition only*@
    };

    static constexpr auto make_begin(EOiterOE first, EOsentOE last) {   // @*exposition only*@
      if constexpr (forward_iterator<EOiterOE>) {
        return utf_iterator{first, first, last};
      } else {
        return utf_iterator{move(first), last};
      }
    }
    static constexpr auto make_end(EOiterOE first, EOsentOE last) {     // @*exposition only*@
      if constexpr (!same_as<EOiterOE, EOsentOE>) {
        return last;
      } else {
        return utf_iterator{move(first), last, last};
      }
    }

    V EObase_OE = V();                                          // @*exposition only*@

  public:
    constexpr utf_view() requires default_initializable<V>
    = default;
    constexpr utf_view(V base) : EObase_OE{move(base)} { }

    constexpr V base() const& requires copy_constructible<V>
    {
      return EObase_OE;
    }
    constexpr V base() && { return move(EObase_OE); }

    constexpr auto begin() requires (!copyable<EOiterOE>)
    {
      return make_begin(ranges::begin(EObase_OE), ranges::end(EObase_OE));
    }
    constexpr auto begin() const requires copyable<EOiterOE>
    {
      return make_begin(ranges::begin(EObase_OE), ranges::end(EObase_OE));
    }

    constexpr auto end() requires (!copyable<EOiterOE>)
    {
      return make_end(ranges::begin(EObase_OE), ranges::end(EObase_OE));
    }
    constexpr auto end() const requires copyable<EOiterOE>
    {
      return make_end(ranges::begin(EObase_OE), ranges::end(EObase_OE));
    }

    constexpr bool empty() const { return ranges::empty(EObase_OE); }

    friend ostream& operator<<(ostream& os, utf_view v) { }
    friend wostream& operator<<(wostream& os, utf_view v) { }

    // todo friendship with other utf_view/iterator
  };

} // namespace p2728
