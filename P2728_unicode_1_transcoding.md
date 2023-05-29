---
title: "Unicode in the Library, Part 1: UTF Transcoding"
document: P2728R3
date: 2023-05-04
audience:
  - SG-16 Unicode
  - LEWG-I
  - LEWG
author:
  - name: Zach Laine
    email: <whatwasthataddress@gmail.com>
toc: true
monofont: "DejaVu Sans Mono"

---

# Changelog

## Changes since R0

- When naming code points in interfaces, use `char32_t`.
- When naming code units in interfaces, use `charN_t`.
- Remove each eager algorithm, leaving in its corresponding view.
- Remove all the output iterators.
- Change template parameters to `utfN_view` to the types of the from-range,
  instead of thetypes of the transcoding iterators used to implement the view.
- Remove all make-functions.
- Replace the misbegotten `as_utfN()` functions with the `as_utfN` view
  adaptors that should have been there all along.
- Add missing `transcoding_error_handler` concept.
- Turn `unpack_iterator_and_sentinel` into a CPO.
- Lower the UTF iterator concepts from bidirectional to input.

## Changes since R1

- Reintroduce the transcoding-from-a-buffer example.
- Generalize `null_sentinel_t` to a non-Unicode-specific facility.
- In utility functions that search for ill-formed encoding, take a range
  argument instead of a pair of iterator arguments.
- Replace `utf{8,16,32}_view` with a single `utf_view`.

## Changes since R2

- Add `noexcept` where appropriate.
- Remove non-essential constants and utility functions, and elaborate on the
  usage of the ones that remain.
- Note differences from similar elements proposed in [@P1629R1].
- Extend the examples slightly.
- Correct an error in the description of the view adaptors' semantics, and
  provide several examples of their use.

# Motivation

Unicode is important to many, many users in everyday software.  It is not
exotic or weird.  Well, it's weird, but it's not weird to see it used.  C and
C++ are the only major production languages with essentially no support for
Unicode.

Let's fix.

To fix, first we start with the most basic representations of strings in
Unicode: UTF.  You might get a UTF string from anywhere; on Windows you often
get them from the OS, in UTF-16.  In web-adjacent applications, strings are
most commonly in UTF-8.  In ASCII-only applications, everything is in UTF-8,
by its definition as a superset of ASCII.

Often, an application needs to switch between UTFs: 8 -> 16, 32 -> 16, etc.
In SG-16 we've taken to calling such UTF-N -> UTF-M operations "transcoding".

I'm proposing interfaces to do transcoding that meet certain design
requirements that I think are important; I hope you'll agree:

- Ranges are the future.  We should have range-friendly ways of doing
  transcoding.  This includes support for sentinels and lazy views.
- Iterators are the present.  We should support generic programming, whether
  it is done in terms of pointers, a particular iterator, or an iterator type
  specified as a template parameter.
- A null-terminated string should not be treated as a special case.  The
  ubiquity of such strings means that they should be treated as first-class
  strings.
- It is common to want to view the same text as code points and code units at
  different times.  It is therefore important that transcoding iterators have
  a convenient way to access the underlying sequence of code units being
  transcoded.

## A note about P1629

[@P1629R1] from JeanHeyd Meneide is a much more ambitious proposal that aims
to standardize a general-purpose text encoding conversion mechanism.  This
proposal is not at odds with P1629; the two proposals have largely orthogonal
aims.  This proposal only concerns itself with UTF interconversions, which is
all that is required for Unicode support.  P1629 is concerned with those
conversions, plus a lot more.  Accepting both proposals would not cause
problems; in fact, the APIs proposed here could be used to implement parts of
the P1629 design.

There are some differences between the way that the transcode views and
iterators from [@P1629R1] work and the transcoding view and iterators from
this paper work.  First, `std::text::transcode_view` has no direct support for
null-terminated strings.  Second, it does not do the unpacking described in
this paper.  Third, they are not printable and streamable.

# The shortest Unicode primer imaginable

There are multiple encoding types defined in Unicode: UTF-8, UTF-16, and
UTF-32.

A *code unit* is the lowest-level datum-type in your Unicode data. Examples
are a `char` in UTF-8 and a `char32_t` in UTF-32.

A *code point* is a 32-bit integral value that represents a single Unicode
value. Examples are U+0041 "A" "LATIN CAPITAL LETTER A" and U+0308 "¨"
"COMBINING DIAERESIS".

A code point may be consist of multiple code units.  For instance, 3 UTF-8
code units in sequence may encode a particular code point.

# Use cases

## Case 1: Adapt to an existing range interface taking a different UTF

In this case, we have a generic range interface to transcode into, so we use a
transcoding view.

```cpp
// A generic function that accepts sequences of UTF-16.
template<std::uc::utf16_range R>
void process_input(R r);
void process_input_again(std::uc::utf_view<std::uc::format::utf16, std::ranges::ref_view<std::string>> r);

std::u8string input = get_utf8_input();
auto input_utf16 = std::views::all(input) | std::uc::as_utf16;

process_input(input_utf16);
process_input_again(input_utf16);
```

## Case 2: Adapt to an existing iterator interface taking a different UTF

This time, we have a generic iterator interface we want to transcode into, so
we want to use the transcoding iterators.

```cpp
// A generic function that accepts sequences of UTF-16.
template<std::uc::utf16_iter I>
void process_input(I first, I last);

std::u8string input = get_utf8_input();

process_input(std::uc::utf_8_to_16_iterator(input.begin(), input.begin(), input.end()),
              std::uc::utf_8_to_16_iterator(input.begin(), input.end(), input.end()));

// Even more conveniently:
auto const utf16_view = std::views::all(input) | std::uc::as_utf16;
process_input(utf16_view.begin(), utf16.end());
```

## Case 3: Print the results of transcoding

Text processing is pretty useless without I/O.  All of the Unicode algorithms
operate on code points, and so the output of any of those algorithms will be
in code points/UTF-32.  It should be easy to print the results to a
`std::ostream`, to a `std::wostream` on Windows, or using `std::print`.
`utf_view` is therefore printable and streamable.

```c++
void double_print(char32_t const * str)
{
    auto utf8 = str | std::uc::as_utf8;
    std::print("{}", utf8);
    std::cerr << utf8;
}
```

# Proposed design

## Dependencies

This proposal depends on the existence of
[P2727](https://isocpp.org/files/papers/P2727R0.html)
"std::iterator_interface".

## Add concepts that describe parameters to transcoding APIs

```cpp
namespace std::uc {

  enum class format { utf8 = 1, utf16 = 2, utf32 = 4 };

  template<class T, format F>
    concept code_unit = integral<T> && sizeof(T) == (int)F;

  template<class T>
    concept utf8_code_unit = code_unit<T, format::utf8>;

  template<class T>
    concept utf16_code_unit = code_unit<T, format::utf16>;

  template<class T>
    concept utf32_code_unit = code_unit<T, format::utf32>;

  template<class T>
    concept utf_code_unit = utf8_code_unit<T> || utf16_code_unit<T> || utf32_code_unit<T>;

  template<class T, format F>
    concept code_unit_iter =
      input_iterator<T> && code_unit<iter_value_t<T>, F>;
  template<class T, format F>
    concept code_unit_pointer =
      is_pointer_v<T> && code_unit<iter_value_t<T>, F>;
  template<class T, format F>
    concept code_unit_range = ranges::input_range<T> &&
      code_unit<ranges::range_value_t<T>, F>;

  template<class T>
    concept utf8_iter = code_unit_iter<T, format::utf8>;
  template<class T>
    concept utf8_pointer = code_unit_pointer<T, format::utf8>;
  template<class T>
    concept utf8_range = code_unit_range<T, format::utf8>;

  template<class T>
    concept utf16_iter = code_unit_iter<T, format::utf16>;
  template<class T>
    concept utf16_pointer = code_unit_pointer<T, format::utf16>;
  template<class T>
    concept utf16_range = code_unit_range<T, format::utf16>;

  template<class T>
    concept utf32_iter = code_unit_iter<T, format::utf32>;
  template<class T>
    concept utf32_pointer = code_unit_pointer<T, format::utf32>;
  template<class T>
    concept utf32_range = code_unit_range<T, format::utf32>;

  template<class T>
    concept utf_iter = utf8_iter<T> || utf16_iter<T> || utf32_iter<T>;
  template<class T>
    concept utf_pointer = utf8_pointer<T> || utf16_pointer<T> || utf32_pointer<T>;
  template<class T>
    concept utf_range = utf8_range<T> || utf16_range<T> || utf32_range<T>;

  template<class T>
    concept utf_range_like =
      utf_range<remove_reference_t<T>> || utf_pointer<remove_reference_t<T>>;

  template<class T>
    concept utf8_input_range_like =
      (ranges::input_range<remove_reference_t<T>> && utf8_code_unit<iter_value_t<T>>) ||
      utf8_pointer<remove_reference_t<T>>;
  template<class T>
    concept utf16_input_range_like =
      (ranges::input_range<remove_reference_t<T>> && utf16_code_unit<iter_value_t<T>>) ||
      utf16_pointer<remove_reference_t<T>>;
  template<class T>
    concept utf32_input_range_like =
      (ranges::input_range<remove_reference_t<T>> && utf32_code_unit<iter_value_t<T>>) ||
      utf32_pointer<remove_reference_t<T>>;

  template<class T>
    concept utf_input_range_like =
      utf8_input_range_like<T> || utf16_input_range_like<T> || utf32_input_range_like<T>;

  template<class T>
    concept transcoding_error_handler =
      requires (T t, char const * msg) { { t(msg) } -> code_point; };

}
```

## Add a standard null-terminated sequence sentinel

```cpp
namespace std {
  struct null_sentinel_t {
    constexpr null_sentinel_t base() const noexcept { return {}; }

    template<class T>
      friend constexpr bool operator==(const T* p, null_sentinel_t)
        { return *p == T{}; }
  };

  inline constexpr null_sentinel_t null_sentinel;
}
```

The `base()` member bears explanation.  It is there to make iterator/sentinel
pairs easy to use in a generic context.  Consider a range `r1` of code points
delimited by a pair of `utf_8_to_32_iterator<char const *>` transcoding
iterators (defined later in this paper).  The range of underlying UTF-8 code
units is [`r1.begin().base()`, `r1.end().base()`).

Now consider a range `r2` of code points that is delimited by a
`utf_8_to_32_iterator<char const *>` transcoding iterator and a
`null_sentinel`.  Now our underlying range of UTF-8 is [`r.begin().base()`,
`null_sentinel`).

Instead of making people writing generic code have to special-case the use of
`null_sentinel`, `null_sentinel` has a `base()` member that lets us write
`r.end().base()` instead of `null_sentinel`.  This means that for either `r` or
`r2`, the underlying range of UTF-8 code units is just [`r1.begin().base()`,
`r1.end().base()`).

Note that this is a general-interest utility, and as such, it is in `std`, not
`std::uc`.

## Add the transcoding iterator template

I'm using [P2727](https://isocpp.org/files/papers/P2727R0.html)'s
`iterator_interface` here for simplicity.

```cpp
namespace std::uc {
  template<class I>
  constexpr auto @*bidirectional-at-most*@() {    // @*exposition only*@
    if constexpr (bidirectional_iterator<I>) {
      return bidirectional_iterator_tag{};
    } else if constexpr (forward_iterator<I>) {
      return forward_iterator_tag{};
    } else if constexpr (input_iterator<I>) {
      return input_iterator_tag{};
    }
  }
  
  template<class I>
  using @*bidirectional-at-most-t*@ = decltype(@*bidirectional-at-most*@<I>()); // @*exposition only*@

  template<format Format>
  constexpr auto @*format-to-type*@() {           // @*exposition only*@
    if constexpr (Format == format::utf8) {
      return char8_t{};
    } else if constexpr (Format == format::utf16) {
      return char16_t{};
    } else {
      return char32_t{};
    }
  }

  template<class I>
  using @*format-to-type-t*@ = decltype(@*format-to-type*@<I>()); // @*exposition only*@

  template<typename I, bool SupportReverse = bidirectional_iterator<I>>
  struct @*first-and-curr*@ {                         // @*exposition only*@
    @*first-and-curr*@() = default;
    @*first-and-curr*@(I curr) : curr{curr} {}
    template<class I2>
      requires convertible_to<I2, I>
        @*first-and-curr*@(const @*first-and-curr*@<I2>& other) : curr{other.curr} {}

    I curr;
  };
  template<typename I>
  struct @*first-and-curr*@<I, true> {                // @*exposition only*@
    @*first-and-curr*@() = default;
    @*first-and-curr*@(I first, I curr) : first{first}, curr{curr} {}
    template<class I2>
      requires convertible_to<I2, I>
        @*first-and-curr*@(const @*first-and-curr*@<I2>& other) : first{other.first}, curr{other.curr} {}

    I first;
    I curr;
  };

  struct use_replacement_character {
    constexpr char32_t operator()(const char*) const noexcept { return replacement_character; }
  };

  template<
    format FromFormat,
    format ToFormat,
    code_unit_iter<FromFormat> I,
    sentinel_for<I> S,
    transcoding_error_handler ErrorHandler>
  class utf_iterator : public iterator_interface<
                         utf_iterator<FromFormat, ToFormat, I, S, ErrorHandler>,
                         @*bidirectional-at-most*@<I>,
                         @*format-to-type-t*@<ToFormat>,
                         @*format-to-type-t*@<ToFormat>> {
  public:
    using value_type = @*format-to-type-t*@<ToFormat>;

    constexpr utf_iterator() = default;

    constexpr utf_iterator(I first, I it, S last) requires bidirectional_iterator<I>
      : first_and_curr_{first, it}, last_(last) {
      if (curr() != last_)
        read();
    }
    constexpr utf_iterator(I it, S last) requires (!bidirectional_iterator<I>)
      : first_and_curr_{it}, last_(last) {
      if (curr() != last_)
        read();
    }

    template<class I2, class S2>
      requires convertible_to<I2, I> && convertible_to<S2, S>
        constexpr utf_iterator(const utf_iterator<FromFormat, ToFormat, I2, S2, ErrorHandler>& other) :
      buf_(other.buf_),
      first_and_curr_(other.first_and_curr_),
      buf_index_(other.buf_index_),
      buf_last_(other.buf_last_),
      last_(other.last_)
    {}

    constexpr I begin() const requires bidirectional_iterator<I> { return first(); }
    constexpr S end() const { return last_; }

    constexpr I base() const requires forward_iterator<I> { return curr(); }

    constexpr value_type operator*() const { return buf_[buf_index_]; }

    constexpr utf_iterator& operator++() {
      if (buf_index_ + 1 == buf_last_ && curr() != last_) {
        if constexpr (forward_iterator<I>) {
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

    constexpr utf_iterator& operator--() requires bidirectional_iterator<I> {
      if (!buf_index_ && curr() != first())
        read_reverse();
      else if (buf_index_)
        --buf_index_;
      return *this;
    }

    friend constexpr bool operator==(utf_iterator lhs, utf_iterator rhs)
      requires forward_iterator<I> || requires (I i) { i != i; } {
      if constexpr (forward_iterator<I>) {
        return lhs.curr() == rhs.curr() && lhs.buf_index_ == rhs.buf_index_;
      } else {
        if (lhs.curr() != rhs.curr())
          return false;

        if (lhs.buf_index_ == rhs.buf_index_ &&
          lhs.buf_last_ == rhs.buf_last_) {
          return true;
        }

        return lhs.buf_index_ == lhs.buf_last_ &&
             rhs.buf_index_ == rhs.buf_last_;
      }
    }

    friend constexpr bool operator==(utf_iterator lhs, S rhs) requires (!same_as<I, S>) {
      if constexpr (forward_iterator<I>) {
        return lhs.curr() == rhs;
      } else {
        return lhs.curr() == rhs && lhs.buf_index_ == lhs.buf_last_;
      }
    }

    using base_type =                   // @*exposition only*@
      iterator_interface<
        utf_iterator<FromFormat, ToFormat, I, S, ErrorHandler>,
        @*bidirectional-at-most-t*@<I>,
        value_type,
        value_type>;
    using base_type::operator++;
    using base_type::operator--;

  private:
    constexpr void read();                                            // @*exposition only*@
    constexpr void read_reverse();                                    // @*exposition only*@

    constexpr I first() const requires bidirectional_iterator<I>      // @*exposition only*@
      { return first_and_curr_.first; }
    constexpr I& curr() { return first_and_curr_.curr; }              // @*exposition only*@
    constexpr I curr() const { return first_and_curr_.curr; }         // @*exposition only*@

    array<value_type, 4 / static_cast<int>(ToFormat)> buf_;           // @*exposition only*@

    @*first-and-curr*@<I> first_and_curr_;                                // @*exposition only*@

    uint8_t buf_index_ = 0;                                           // @*exposition only*@
    uint8_t buf_last_ = 0;                                            // @*exposition only*@
    uint8_t to_increment_ = 0;                                        // @*exposition only*@

    [[no_unique_address]] S last_;                                    // @*exposition only*@

    template<
      format FromFormat2,
      format ToFormat2,
      code_unit_iter<FromFormat2> I2,
      sentinel_for<I2> S2,
      transcoding_error_handler ErrorHandler2>
    friend class utf_iterator;
  };
}
```

`use_replacement_character` is an error handler type that can be used with
`utf_iterator`.  It accepts a `const char *` error message, and returns the
replacement character.  The user can subtitute their own type here, which may
throw, abort, log, etc.

`utf_iterator` is an iterator that transcodes from UTF-N to UTF-M, where N and
M are each one of 8, 16, or 32.  Note that N can equal M.  UTF-N to UTF-N
operation can be used to ensure correct encoding without changing format.
`utf_iterator` does its work by adapting an underlying range of code units.
Each code point `c` to be transcoded is decoded from `FromFormat` in the
underlying range.  `c` is then encoded to `ToFormat` into an internal buffer.
If ill-formed UTF is encountered during the decoding step, `c` is whatever
`ErrorHandler{}("")` returns; using the default error handler, this is
`replacement_character`.

`utf_iterator` maintains certain invariants; the invariants differ based on
whether `utf_iterator` is an input iterator.

For input iterators the invairants are that either: `*this` is at the end of
the range being adapted, and `curr()` == `last_`; or the position of `curr()`
is always at the end of the current code point `c` within the range being
adapted, and `buf_` contains the code units in `ToFormat` that comprise `c`.

For forward and bidirectional iterators, the invariants are either: `*this` is
at the end of the range being adapted, and `curr()` == `last_`; or the
position of `curr()` is always at the beginning of the current code point `c`
within the range being adapted, and `buf_` contains the code units in
`ToFormat` that comprise `c`.

When ill-formed UTF is encountered in the range being adapted, `utf_iterator`
calls `ErrorHandler{}.operator()` to produce a character to represent the
ill-formed sequence.  The number and position of error handler invocations
within the transcoded output is the same, whether the range being adapted is
traversed forward or backward.  The number and position of the error handler
invocations should use the "substitution of maximal subparts" approach
described in Chapter 3 of the Unicode standard.

Besides the constructors, no member function of `utf_iterator` has
preconditions.  As long as a `utf_iterator` `i` is constructed with proper
arguments, all subsequent operations on `i` are memory safe.  This includes
decrementing a `utf_iterator` at the beginning of the range being adapted, and
incrementing or dereferencing a `utf_iterator` at the end of the range being
adapted.

If `FromFormat` and `ToFormat` are not each one of `format::utf8`,
`format::utf16`, or `format::utf32`, the program is ill-formed.

If `input_iterator<I>` is `true`, `noexcept(ErrorHandler{}("")))` must be
`true` as well; otherwise, the program is ill-formed.

The exposition-only member function `read` decodes the code point `c` as
`FromFormat` starting from position `curr()` in the range being adapted (`c`
may be `replacement_character`); sets `to_increment_` to the number of code
units read while decoding `c`; encodes `c` as `ToFormat` into `buf_`; sets
`buf_index_` to `0`; and sets `buf_last_` to the number of code units encoded
into `buf_`.  If `forward_iterator<I>` is `true`, `curr()` is set to the
position it had before `read` was called.  If an exception is thrown during a
call to `read`, the call to `read` has no effect.

The exposition-only member function `read_reverse` decodes the code point `c`
as `FromFormat` ending at position `curr()` in the range being adapted (`c`
may be `replacement_character`); sets `to_increment_` to the number of code
units read while decoding `c`; encodes `c` as `ToFormat` into `buf_`; sets
`buf_last_` to the number of code units encoded into `buf_`; and sets
`buf_index_` to `buf_last_ - 1`.  If an exception is thrown during a call to
`read_reverse`, the call to `read_reverse` has no effect.

# TODO: Explain why forward iterators are not unpackable

## Add a transcoding view and adaptors

```cpp
namespace std::uc {
  template<typename T>
  constexpr format @*format-of*@() {              // @*exposition only*@
    if constexpr (same_as<T, char8_t>) {
      return format::utf8;
    } else if constexpr (same_as<T, char16_t>) {
      return format::utf16;
    } else {
      return format::utf8;
    }
  }

  template<format Format, ranges::view V>
  class utf_view : public ranges::view_interface<utf_view<Format, V>> {
    V base_ = V();                                // @*exposition only*@

  public:
    constexpr utf_view() requires default_initializable<V> = default;
    constexpr utf_view(V base) : base_{std::move(base)} {}

    constexpr V base() const & requires copy_constructible<V> { return base_; }
    constexpr V base() && { return std::move(base_); }

    constexpr auto begin() const {
      constexpr format from_format = @*format-of*@<ranges::range_value_t<V>>();
      if constexpr (ranges::bidirectional_range<V>) {
        return utf_iterator<from_format, Format, ranges::iterator_t<V>, ranges::sentinel_t<V>>{
          ranges::begin(base_),
          ranges::begin(base_),
          ranges::end(base_)};
      } else {
        return utf_iterator<from_format, Format, ranges::iterator_t<V>, ranges::sentinel_t<V>>{
          ranges::begin(base_), ranges::end(base_)};
      }
    }
    constexpr auto end() const {
      constexpr format from_format = @*format-of*@<ranges::range_value_t<V>>();
      if constexpr (!ranges::common_range<V>) {
        return ranges::end(base_);
      } else if constexpr (ranges::bidirectional_range<V>) {
        return utf_iterator<from_format, Format, ranges::iterator_t<V>, ranges::sentinel_t<V>>{
          ranges::begin(base_),
          ranges::end(base_),
          ranges::end(base_)};
      } else {
        return utf_iterator<from_format, Format, ranges::iterator_t<V>, ranges::sentinel_t<V>>{
          ranges::end(base_), ranges::end(base_)};
      }
    }

    friend ostream & operator<<(ostream & os, utf_view v) {
      if constexpr (Format == format::utf8) {
        auto out = ostreambuf_iterator<char>(os);
        for (auto it = v.begin(); it != v.end(); ++it, ++out) {
          *out = *it;
        }
      } else {
        boost::text::transcode_to_utf8(
          v.begin(), v.end(), ostreambuf_iterator<char>(os));
      }
      return os;
    }
    friend wostream & operator<<(wostream & os, utf_view v) {
      if constexpr (Format == format::utf16) {
        auto out = ostreambuf_iterator<wchar_t>(os);
        for (auto it = v.begin(); it != v.end(); ++it, ++out) {
          *out = *it;
        }
      } else {
        boost::text::transcode_to_utf16(
          v.begin(), v.end(), ostreambuf_iterator<wchar_t>(os));
      }
      return os;
    }
  };

  template<utf_range V>
    requires ranges::view<V>
  class utf8_view : public utf_view<format::utf8, V> {
  public:
    constexpr utf8_view() requires default_initializable<V> = default;
    constexpr utf8_view(V base) : utf_view<format::utf8, V>{std::move(base)} {}
  };
  template<utf_range V>
    requires ranges::view<V>
  class utf16_view : public utf_view<format::utf16, V> {
  public:
    constexpr utf16_view() requires default_initializable<V> = default;
    constexpr utf16_view(V base) : utf_view<format::utf16, V>{std::move(base)} {}
  };
  template<utf_range V>
    requires ranges::view<V>
  class utf32_view : public utf_view<format::utf32, V> {
  public:
    constexpr utf32_view() requires default_initializable<V> = default;
    constexpr utf32_view(V base) : utf_view<format::utf32, V>{std::move(base)} {}
  };

  template<class R>
  utf8_view(R &&) -> utf8_view<views::all_t<R>>;
  template<class R>
  utf16_view(R &&) -> utf16_view<views::all_t<R>>;
  template<class R>
  utf32_view(R &&) -> utf32_view<views::all_t<R>>;
}

namespace std::ranges {
    template<uc::format Format, class V>
    inline constexpr bool enable_borrowed_range<uc::utf_view<Format, V>> = enable_borrowed_range<V>;
    template<class V>
    inline constexpr bool enable_borrowed_range<uc::utf8_view<V>> = enable_borrowed_range<V>;
    template<class V>
    inline constexpr bool enable_borrowed_range<uc::utf16_view<V>> = enable_borrowed_range<V>;
    template<class V>
    inline constexpr bool enable_borrowed_range<uc::utf32_view<V>> = enable_borrowed_range<V>;
}

namspace std::uc {
  template<class R>
    constexpr decltype(auto) @*unpack-range*@(R && r) {                     // @*exposition only*@
      using T = remove_cvref_t<R>;
      if constexpr (ranges::forward_range<T>) {
        auto unpacked = uc::unpack_iterator_and_sentinel(ranges::begin(r), ranges::end(r));
        if constexpr (is_bounded_array_v<T>) {
          constexpr auto n = extent_v<T>;
          if (n && !r[n - 1])
            --unpacked.last;
        }
        return ranges::subrange(unpacked.first, unpacked.last);
      } else {
        return forward<R>(r);
      }
    }

  inline constexpr @*unspecified*@ as_utf8;
  inline constexpr @*unspecified*@ as_utf16;
  inline constexpr @*unspecified*@ as_utf32;
}
```

# TODO: Explain why utf_view always uses utf_iterator, even in utfN -> utfN cases.

`utf_view` produces a view in UTF format `Format` of the elements from another
UTF view.  `utf8_view` produces a UTF-8 view of the elements from another UTF
view.  `utf16_view` produces a UTF-16 view of the elements from another UTF
view.  `utf32_view` produces a UTF-32 view of the elements from another UTF
view.  Let `utfN_view` denote any one of the views `utf8_view`, `utf16_view`,
amd `utf32_view`.

The names `as_utf8`, `as_utf16`, and `as_utf32` denote range adaptor objects
([range.adaptor.object]).  `as_utf8` produces `utf8_view`s, `as_utf16`
produces `utf16_view`s, and `as_utf32` produces `utf32_view`s.  Let `as_utfN`
denote any one of `as_utf8`, `as_utf16`, and `as_utf32`, and let `V` denote
the `utfN_view` associated with that object.  Let `E` be an expression and let
`T` be `remove_cvref_t<decltype((E))>`.  If `decltype((E))` does not model
`utf_range_like`, `as_utfN(E)` is ill-formed.  The expression `as_utfN(E)` is
expression-equivalent to:

- If `T` is a specialization of `empty_view` ([range.empty.view]), then
  `@*decay-copy*@(E)`.

- Otherwise, if `is_pointer_v<T>` is `true`, then
  `V(ranges::subrange(E, null_sentinel))`.

- Otherwise, `V(@*unpack-range*@(E))`.

\[Example 1:
```c++
std::u32string s = U"Unicode";
for (char8_t c : s | as_utf8)
  cout << (char)c << ' '; // prints U n i c o d e 
```
— end example\]

\[Example 2:
```c++
auto * s = L"is weird.";
for (char8_t c : s | as_utf8)
  cout << (char)c << ' '; // prints i s   w e i r d . 
```
— end example\]

The `ostream` and `wostream` stream operators transcode the `utf_view` to
UTF-8 and UTF-16 respectively (if transcoding is needed), and the `wostream`
overload is only defined on Windows.

More examples:

```c++
static_assert(std::is_same_v<
              decltype(u8"text" | std::uc::as_utf16),
              std::uc::utf16_view<std::ranges::subrange<const char8_t*, const char8_t*>>>);

std::u8string str = u8"text";

static_assert(std::is_same_v<
              decltype(str | std::uc::as_utf16),
              std::uc::utf16_view<std::ranges::subrange<std::u8string::iterator>>>);

static_assert(std::is_same_v<
              decltype(str.c_str() | std::uc::as_utf16),
              std::uc::utf16_view<std::ranges::subrange<const char8_t *, std::uc::null_sentinel_t>>>);

static_assert(std::is_same_v<
              decltype(std::ranges::empty_view<int>{} | std::uc::as_utf16),
              std::ranges::empty_view<int>>);

std::u16string str2 = u"text";

static_assert(std::is_same_v<
              decltype(str2 | std::uc::as_utf16),
              std::uc::utf16_view<std::ranges::subrange<std::u16string::iterator>>>);

static_assert(std::is_same_v<
              decltype(str2.c_str() | std::uc::as_utf16),
              std::uc::utf16_view<std::ranges::subrange<const char16_t *, std::uc::null_sentinel_t>>>);
```

### Add `utf_view` specialization of `formatter`

These should be added to the list of "the debug-enabled string type
specializations" in [format.formatter.spec].  This allows `utf_view` and
`utfN_view` to be used in `std::format()` and `std::print()`.  The intention
is that the formatter will transcode to UTF-8 if the formatter's `charT` is
`char`, or to UTF-16 or UTF-32 (which one is implementation defined) if the
formatter's `charT` is `wchar_t` -- if transcoding is necessary at all.

```cpp
namespace std {
  template<uc::format Format, class V, class CharT>
  struct formatter<uc::utf_view<Format, V>, CharT> {
  private:
    formatter<basic_string<charT>, charT> @*underlying_*@;                // @*exposition only*@

  public:
    template<class ParseContext>
      constexpr typename ParseContext::iterator
        parse(ParseContext& ctx);

    template<class FormatContext>
      typename FormatContext::iterator
        format(const uc::utf_view<Format, V>& view, FormatContext& ctx) const;

    constexpr void set_debug_format() noexcept;
  };

  template<class V, class CharT>
  struct formatter<uc::utf8_view<V>, CharT> : formatter<uc::utf_view<uc::format::utf8, V>, CharT> {
    template<class FormatContext>
    auto format(const uc::utf8_view<V>& view, FormatContext& ctx) const {
      return formatter<uc::utf_view<uc::format::utf8, V>,CharT>::format(view, ctx);
    }
  };

  template<class V, class CharT>
  struct formatter<uc::utf16_view<V>, CharT> : formatter<uc::utf_view<uc::format::utf16, V>, CharT> {
    template<class FormatContext>
    auto format(const uc::utf16_view<V>& view, FormatContext& ctx) const {
      return formatter<uc::utf_view<uc::format::utf16, V>,CharT>::format(view, ctx);
    }
  };

  template<class V, class CharT>
  struct formatter<uc::utf32_view<V>, CharT> : formatter<uc::utf_view<uc::format::utf32, V>, CharT> {
    template<class FormatContext>
    auto format(const uc::utf32_view<V>& view, FormatContext& ctx) const {
      return formatter<uc::utf_view<uc::format::utf32, V>,CharT>::format(view, ctx);
    }
  };
}
```

```c++
template<class ParseContext>
  constexpr typename ParseContext::iterator
    parse(ParseContext& ctx);
```

Effects: Equivalent to:
```c++
return @*underlying_*@.parse(ctx);
```

Returns: An iterator past the end of the range-format-spec.

```c++
template<class FormatContext>
  typename FormatContext::iterator
    format(const uc::utf_view<Format, V>& view, FormatContext& ctx) const;
```

Effects: Equivalent to:
```c++
auto adaptor = @*see below*@;
return @*underlying_*@.format(basic_string<charT>(from_range, view | adaptor, ctx);
```

`adaptor` is `uc::as_utf8` if `charT` is `char`.  Otherwise, it is
implementation defined whether `adaptor` is `uc::as_utf16` or `uc::as_utf32`.

### Add `unpack_iterator_and_sentinel` CPO for iterator "unpacking"

```cpp
struct no_op_repacker {
  template<class T>
    T operator()(T x) const { return x; }
};

template<class RepackedIterator, class I, class S, class Then>
struct repacker {
  auto operator()(I it) const { return then(RepackedIterator(first, it, last)); }

  I first;
  [[no_unique_address]] S last;
  [[no_unique_address]] Then then;
};

template<format FormatTag, utf_iter I, sentinel_for<I> S, class Repack>
struct utf_tagged_range {
  static constexpr format format_tag = FormatTag;

  I first;
  [[no_unique_address]] S last;
  [[no_unique_address]] Repack repack;
};

// CPO equivalent to:
template<utf_iter I, sentinel_for<I> S, class Repack = no_op_repacker>
constexpr auto unpack_iterator_and_sentinel(I first, S last, Repack repack = Repack());
```

A simple way to represent a transcoding view is as a pair of transcoding
iterators. However, there is a problem with that approach, since a
`utf_view<format::utf32, utf_8_to_32_iterator<char const *>>` would be a range the size of
6 pointers. Worse yet, a
`utf_view<format::utf32, utf_8_to_16_iterator<utf_16_to_32_iterator<char const *>>>` would
be the size of 18 pointers! Further, such a view would do a UTF-8 to UTF-16 to
UTF-32 conversion, when it could have done a direct UTF-8 to UTF-32 conversion
instead.

To solve these kinds of problems, `utf_view` unpacks the iterators it is given
in the view it adapts, so that only the bottom-most underlying pointer or
iterator is stored:

```cpp
std::string str = "some text";

auto to_16_first = std::uc::utf_8_to_16_iterator<std::string::iterator>(
    str.begin(), str.begin(), str.end());
auto to_16_last = std::uc::utf_8_to_16_iterator<std::string::iterator>(
    str.begin(), str.end(), str.end());

auto to_32_first = std::uc::utf_16_to_32_iterator<
    std::uc::utf_8_to_16_iterator<std::string::iterator>
>(to_16_first, to_16_first, to_16_last);
auto to_32_last = std::uc::utf_16_to_32_iterator<
    std::uc::utf_8_to_16_iterator<std::string::iterator>
>(to_16_first, to_16_last, to_16_last);

auto range = std::ranges::subrange(to_32_first, to_32_last) | std::uc::as_utf8;

// Poof!  The utf_16_to_32_iterators disappeared!
static_assert(std::is_same<std::ranges::iterator_t<decltype(range)>, std::string::iterator>::value, "");
```

Each of these views stores only a single iterator and sentinel, so each view
is typically the size of two pointers, and possibly smaller if a sentinel is
used.

The same unpacking logic is used in the entire proposed API.  This allows you
to write `r | std::uc::as_utf32` in a generic context, without caring whether
`r` is a range of UTF-8, UTF-16, or UTF-32. You do not need to care about
whether `r` is a common range or not.  You also can ignore whether `r` is
comprised of raw pointers, some other kind of iterator, or transcoding
iterators. For example, if `r.begin()` is a `utf_32_to_8_iterator`, the
resulting view will use `r.begin().base()` for its begin-iterator.

Sometimes, an interface might accept any UTF-N iterator, and then transcode
internally to UTF-32:

```c++
template<input_iterator I, sentinel_for<I> S, output_iterator<char8_t> O>
  requires (utf8_code_unit<iter_value_t<I>> || utf16_code_unit<iter_value_t<I>>)
transcode_result<I, O> transcode_to_utf32(I first, S last, O out);
```

For such interfaces, it can be difficult in the general case to form an
iterator of type `I` to return to the user:

```c++
template<input_iterator I, sentinel_for<I> S, output_iterator<char8_t> O>
    requires (utf8_code_unit<iter_value_t<I>> || utf16_code_unit<iter_value_t<I>>)
transcode_result<I, O> transcode_to_utf32(I first, S last, O out) {
    // Get the input as UTF-32.
    auto r = uc::utf_view(uc::format::utf32, first, last);

    // Do transcoding.
    auto copy_result = ranges::copy(r, out);

    // Return an in_out_result.
    return result<I, O>{/* ??? */, copy_result.out};
}
```

What should we write for `/* ??? */`?  That is, how do we get back from the
UTF-32 iterator `r.begin()` to an `I` iterator?  It's harder than it first
seems; consider the case where `I` is
`std::uc::utf_16_to_32_iterator<std::uc::utf_8_to_16_iterator<std::string::iterator>>`.
The solution is for the unpacking algorithm to remember the structure of
whatever iterator it unpacks, and then rebuild the structure when returning
the result.  To demonstrate, here is the implementation of
`transcode_to_utf32` from Boost.Text:

```c++
template<std::input_iterator I, std::sentinel_for<I> S, std::output_iterator<char32_t> O>
    requires (utf8_code_unit<std::iter_value_t<I>> || utf16_code_unit<std::iter_value_t<I>>)
transcode_result<I, O> transcode_to_utf32(I first, S last, O out)
{
    auto const r = boost::text::unpack_iterator_and_sentinel(first, last);
    auto unpacked = detail::transcode_to_32<false>(
        detail::tag_t<r.format_tag>, r.first, r.last, -1, out);
    return {r.repack(unpacked.in), unpacked.out};
}
```

If this all sounds way too complicated, it's not that bad at all.  Here's the
unpacking/repacking implementation from Boost.Text:
[unpack.hpp](https://github.com/tzlaine/text/blob/develop/include/boost/text/unpack.hpp).

`unpack_iterator_and_sentinel` is a CPO.  It is intended to work with UDTs
that provide ther own unpacking implementation.  It returns a
`utf_tagged_range`.

## Add a feature test macro

Add the feature test macro `__cpp_lib_unicode_transcoding`.

## Design notes

None of the proposed interfaces is subject to change in future versions of
Unicode; each relates to the guaranteed-stable subset.  Just sayin'.

None of the proposed interfaces allocates.

The proposed interfaces allow users to choose amongst multiple
convenience-vs-compatibility tradeoffs.  Explicitly, they are:

- If you need compatibility with existing iterator-based algorithms (such as
  the standard algorithms), use the transcoding iterators.
- If you want streamability or the convenience of constructing ranges with a
  single `| as_utfN` adaptor use, use the transcoding views.

All the transcoding iterators allow you access to the underlying iterator via
`.base()`, following the convention of the iterator adaptors already in the
standard.

The transcoding views are lazy, as you'd expect.  They also compose with the
standard view adaptors, so just transcoding at most 10 UTF-16 code units out
of some UTF can be done with `foo | std::uc::as_utf16 |
std::ranges::views::take(10)`.

Error handling is explicitly configurable in the transcoding iterators.  This
gives complete control to those who want to do something other than the
default.  The default, according to Unicode, is to produce a replacement
character (`0xfffd`) in the output when broken UTF encoding is seen in the
input.  This is what all these interfaces do, unless you configure one of the
iterators as mentioned above.

The production of replacement characters as error-handling strategy is good
for memory compactness and safety.  It allows us to store all our text as
UTF-8 (or, less compactly, as UTF-16), and then process code points as
transcoding views.  If an error occurs, the transcoding views will simply
produce a replacement character; there is no danger of UB.

Code units are just numbers.  All of these interfaces treat integral types as
code units of various sizes (at least the ones that are 8-, 16-, or 32-bit).
Signedness is ignored.

A null-terminated pointer `p` to an 8-, 16-, or 32-bit string of code units is
considered the implicit range `[p, null_sentinel)`.  This makes user code much
more natural; `"foo" | as_utf16`, `"foo"sv | as_utf16`, and `"foo"s |
as_utf16` are roughly equivalent (though the iterator type of the resulting
view may differ).

Iterators are constructed from more than one underlying iterator.  To do
iteration in many text-handling contexts, you need to know the beginning and
the end of the range you are iterating over, just to be able to do iteration
correctly. Note that this is not a safety issue, but a correctness one.  For
example, say we have a string `s` of UTF-8 code units that we would like to
iterate over to produce UTF-32 code points. If the last code unit in `s` is
`0xe0`, we should expect two more code units to follow. They are not present,
though, because `0xe0` is the last code unit. Now consider how you would
implement `operator++()` for an iterator `iter` that transcodes from UTF-8 to
UTF-32. If you advance far enough to get the next UTF-32 code point in each
call to `operator++()`, you may run off the end of `s` when you find `0xe0`
and try to read two more code units. Note that it does not matter that `iter`
probably comes from a range with an end-iterator or sentinel as its mate;
inside `iter`'s `operator++()` this is no help. `iter` must therefore have the
end-iterator or sentinel as a data member. The same logic applies to the other
end of the range if `iter` is bidirectional — it must also have the iterator
to the start of the underlying range as a data member.  This unfortunate
reality comes up over and over in the proposed iterators, not just the ones
that are UTF transcoding iterators. This is why iterators in this proposal
(and the ones to come) usually consist of three underlying iterators.

# Implementation experience

All the interfaces proposed here have been implemented, and re-implemented,
several times over the last 5 years or so.  They are part of a proposed (but
not yet accepted!) Boost library,
[Boost.Text](https://github.com/tzlaine/text).

The library has hundreds of stars, though I'm not sure how many users that
equates to.  All of the interfaces proposed here are among the best-exercised
in the library.  There are comprehensive tests for all the proposed entities,
and those entities are used as the foundation upon which all the other library
entities are composed.

Though there are a lot of individual entities proposed here, at one time or
another I have need each one of them, though maybe not in every UTF-N -> UTF-M
permutation.  Those transcoding permutations are there mostly for
completeness.  I have only ever needed UTF-8 <-> UTF->32 in any of my work
that uses Unicode.  Frequent Windows users will also need to convert to and
from UTF-16 sometimes, because that is the UTF that the OS APIs use.
