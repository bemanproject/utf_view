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
- Memory safety is important.  Ensuring that the Unicode part of the standard
  library is as meory safe as possible should be a priority.

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
this paper.  Third, it is not printable and streamable.

# The shortest Unicode primer imaginable

There are multiple encoding types defined in Unicode: UTF-8, UTF-16, and
UTF-32.

A *code unit* is the lowest-level datum-type in your Unicode data. Examples
are a `char8_t` in UTF-8 and a `char32_t` in UTF-32.

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

process_input(
    std::uc::utf_iterator<std::uc::format::utf8, std::uc::format::utf16, std::u8string::iterator>(
        input.begin(), input.begin(), input.end()),
    std::uc::utf_iterator<std::uc::format::utf8, std::uc::format::utf16, std::u8string::iterator>(
        input.begin(), input.end(), input.end()));

// Even more conveniently:
auto const utf16_view = input | std::uc::as_utf16;
process_input(utf16_view.begin(), utf16.end());
```

## Case 3: Print the results of transcoding

Text processing is pretty useless without I/O.  All of the Unicode algorithms
operate on code points, and so the output of any of those algorithms will be
in code points/UTF-32.  It should be easy to print the results to a
`std::ostream`, to a `std::wostream` on Windows, or using `std::format` and
`std::print`.  `utf_view` is therefore printable and streamable.

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
      requires (T t, string_view msg) { { t(msg) } -> same_as<char32_t>; };

}
```

## Add a null-terminated sequence sentinel

```cpp
namespace std::uc {
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
delimited by a pair of `utf_iterator<format::utf8, format::utf32, char const
*>` transcoding iterators (defined later in this paper).  The range of
underlying UTF-8 code units is [`r1.begin().base()`, `r1.end().base()`).

Now consider a range `r2` of code points that is delimited by a
`utf_iterator<format::utf8, format::utf32, char const *, null_sentinel_t>`
transcoding iterator and a `null_sentinel`.  Now our underlying range of UTF-8
is [`r.begin().base()`, `null_sentinel`).

Instead of making people writing generic code have to special-case the use of
`null_sentinel`, `null_sentinel` has a `base()` member that lets us write
`r.end().base()` instead of `null_sentinel`.  This means that for either `r` or
`r2`, the underlying range of UTF-8 code units is just [`r1.begin().base()`,
`r1.end().base()`).

::: tonytable

### Without `null_sentinel_t::base()`
```c++
template<typename UTF8To32Iter1, typename UTF8To32Iter2>
auto f(UTF8To32Iter1 first, UTF8To32Iter2 last) {
  if constexpr (std::same_as<UTF8To32Iter1, UTF8To32Iter2>) {
    auto utf8_first = first.base();
    auto utf8_last = last;
    return /* use utf8_{first,last} ... */;
  } else {
    auto utf8_first = first.base();
    auto utf8_last = last.base();
    return /* use utf8_{first,last} ... */;
  }
}
```

### With `null_sentinel_t::base()`
```c++
template<typename UTF8To32Iter1, typename UTF8To32Iter2>
auto f(UTF8To32Iter1 first, UTF8To32Iter2 last) {
  auto utf8_first = first.base();
  auto utf8_last = last.base();
  return /* use utf8_{first,last} ... */;
}
```

:::

Without `null_sentinel_t::base()`, we have to account for the case in which
the null sentinel is passed for the second function parameter.  This makes our
very simple logic for getting the underlying range out of an iterator/sentinel
pair more than twice as long.


## Add the transcoding iterator template

I'm using [P2727](https://isocpp.org/files/papers/P2727R0.html)'s
`iterator_interface` here for simplicity.

First, the synopsis:

```c++
namespace std::uc {
  inline constexpr char32_t replacement_character = 0xfffd;

  struct use_replacement_character {
    constexpr char32_t operator()(string_view error_msg) const noexcept;
  };

  template<
    format FromFormat,
    format ToFormat,
    code_unit_iter<FromFormat> I,
    sentinel_for<I> S = I,
    transcoding_error_handler ErrorHandler = use_replacement_character>
  class utf_iterator;
}
```

Then the definitions:

```c++
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
    constexpr char32_t operator()(string_view) const noexcept { return replacement_character; }
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
`utf_iterator`.  It accepts a `string_view` error message, and returns the
replacement character.  The user can subtitute their own type here, which may
throw, abort, log, etc.

`utf_iterator` is an iterator that transcodes from UTF-N to UTF-M, where N and
M are each one of 8, 16, or 32.  N may equal M.  UTF-N to UTF-N operation
invokes the error handler as appropriate, but does not change format.
`utf_iterator` does its work by adapting an underlying range of code units.
Each code point `c` to be transcoded is decoded from `FromFormat` in the
underlying range.  `c` is then encoded to `ToFormat` into an internal buffer.
If ill-formed UTF is encountered during the decoding step, `c` is whatever
invoking the error handler returns; using the default error handler, this is
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

### Optional: Add aliases for common `utf_iterator` specializations

```c++
namespace std::uc {
    template<
        utf8_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_8_to_16_iterator =
        utf_iterator<format::utf8, format::utf16, I, S, ErrorHandler>;
    template<
        utf16_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_16_to_8_iterator =
        utf_iterator<format::utf16, format::utf8, I, S, ErrorHandler>;

    template<
        utf8_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_8_to_32_iterator =
        utf_iterator<format::utf8, format::utf32, I, S, ErrorHandler>;
    template<
        utf32_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_32_to_8_iterator =
        utf_iterator<format::utf32, format::utf8, I, S, ErrorHandler>;

    template<
        utf16_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_16_to_32_iterator =
        utf_iterator<format::utf16, format::utf32, I, S, ErrorHandler>;
    template<
        utf32_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_32_to_16_iterator =
        utf_iterator<format::utf32, format::utf16, I, S, ErrorHandler>;
}
```

These aliases make it easier to spell `utf_iterator`s.  Consider
`utf_8_to_32_iterator<char const *>` versus `utf_iterator<format::utf8,
format::utf32, char const *>`.  More importantly, they allow CTAD to work, as
in `utf_8_to_32_iterator(first, it, last)`.  These aliases are completely
optional, of course.  Let us poll.

### Add `unpack_iterator_and_sentinel` CPO for iterator "unpacking"

```cpp
struct no_op_repacker {
  template<class T>
    T operator()(T x) const { return x; }
};

template<format FormatTag, utf_iter I, sentinel_for<I> S, class Repack>
struct unpack_result {
  static constexpr format format_tag = FormatTag;

  I first;
  [[no_unique_address]] S last;
  [[no_unique_address]] Repack repack;
};

// CPO equivalent to:
template<utf_iter I, sentinel_for<I> S, class Repack = no_op_repacker>
constexpr auto unpack_iterator_and_sentinel(I first, S last, Repack repack = Repack());
```

Any `utf_iterator` `ti` contains two iterators and a sentinel.  If one were to
adapt `ti` in another transcoding iterator `ti2`, one quickly encounters a
problem -- since for example `utf_iterator<format::utf32, format::utf16,
utf_iterator<format::utf8, format::utf32, char const *>>` would be the size of
9 pointers! Further, such an iterator would do a UTF-8 to UTF-16 to UTF-32
conversion, when it could have done a direct UTF-8 to UTF-32 conversion
instead.

One would obviously never write a type like the monstrosity above.  However,
it is quite possible to accidentally construct one in generic code.  Consider:

```c++
using namespace std::uc;

template<format IterFormat, typename Iter>
void f(Iter it, null_sentinel_t) {
#if _MSC_VER
    // On Windows, do something with 'it' that requires UTF-16.
    utf_iterator<IterFormat, format::utf16, Iter, null_sentinel_t> it16;
    windows_function(it16, null_sentinel);
#endif

    // ... etc.
}

int main(int argc, char const * argv[]) {
    utf_iterator<format::utf8, format::utf32, char const *, null_sentinel_t> it(argv[1], null_sentinel);

    f<format::utf32>(it, null_sentinel);

    // ... etc.
}
```

This example is a bit contrived, since users will not create iterators
directly like this very often.  Users are much more likely to use the
`utfN_view` views and `as_utfN` view adaptors being proposed below.  The view
adaptors are defined in such a way that they avoid this problem altogether.
They do this by unpacking the view they are adapting before adapting it.  For
instance:

```cpp
std::u8string str = u8"some text";

auto utf16_str = str | std::uc::as_utf16;

static_assert(std::same_as<
    decltype(utf16_str.begin()),
    std::uc::utf_iterator<std::uc::format::utf8, std::uc::format::utf16, std::u8string::iterator>
>);

auto utf32_str = utf16_str | std::uc::as_utf32;

// Poof!  The utf_iterator<format::utf8, format::utf16 iterator disappeared!
static_assert(std::same_as<
    decltype(utf32_str.begin()),
    std::uc::utf_iterator<std::uc::format::utf8, std::uc::format::utf32, std::u8string::iterator>
>);
```

The unpacking logic is used in the view adaptors, as shown above.  This allows
you to write `r | std::uc::as_utf32` in a generic context, without caring
whether `r` is a range of UTF-8, UTF-16, or UTF-32. You also do not need to
care about whether `r` is a common range or not.  You also can ignore whether
`r` is comprised of raw pointers, some other kind of iterator, or transcoding
iterators.

This becomes especially useful in the APIs proposed in later papers that
depend on this paper.  In particular, APIs in subsequent papers accept any
UTF-N iterator, and then transcode internally to UTF-32.  However, this
creates a minor problem for some algorithms.  Consider this algorithm (not
proposed) as an example.

```c++
template<input_iterator I, sentinel_for<I> S, output_iterator<char8_t> O>
  requires (utf8_code_unit<iter_value_t<I>> || utf16_code_unit<iter_value_t<I>>)
transcode_result<I, O> transcode_to_utf32(I first, S last, O out);
```

Such a transcoding algorithm is pretty similar to `std::ranges::copy`, in that
you should return both the output iterator *and* the final position of the
input iterator (`transcode_result` is an alias for `in_out_result`).  For such
interfaces, it can be difficult in the general case to form an iterator of
type `I` to return to the user:

```c++
template<input_iterator I, sentinel_for<I> S, output_iterator<char8_t> O>
    requires (utf8_code_unit<iter_value_t<I>> || utf16_code_unit<iter_value_t<I>>)
transcode_result<I, O> transcode_to_utf32(I first, S last, O out) {
    // Get the input as UTF-32.  This may involve unpacking, so possibly decltype(r.begin()) != I.
    auto r = ranges::subrange(first, last) | uc::as_utf32;

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

Note the call to `r.repack`.  This is an invocable created by the unpacking
process itself.

If this all sounds way too complicated, it's not that bad at all.  Here's the
unpacking/repacking implementation from Boost.Text:
[unpack.hpp](https://github.com/tzlaine/text/blob/develop/include/boost/text/unpack.hpp).

`unpack_iterator_and_sentinel` is a CPO.  It is intended to work with UDTs
that provide their own unpacking implementation.  It returns an
`unpack_result`.

### Why input iterators are not unpackable

Input iterators are messed up.  They barely resemble the other iterators.  For
one thing, they are single-pass.  This means that when a `utf_iterator`
adapting an input iterator reads the next code point from the range it is
adapting, it must leave the iterator at a location that is just after the
current code point.  It has no choice, since it cannot backtrack.

It is possible to unpack an input iterator in an entirely different way than
other iterators.  The unpack operation for input iterators could be to produce
the underlying code unit iterator (the adapted input iterator itself), *plus*
the current code point that the input iterator was just used to read.

However, this is not very much help.  Consider a case in which we need to
unpack a UTF-8 to UTF-32 transcoding iterator so we can form a UTF-8 to UTF-16
iterator instead.  The unpack operation will produce an unpacked input
transcoding iterator -- the moral equivalent of `std::pair<I, char32_t>`.

What can you do with this?  Well, you can try to construct a
`utf_iterator<format::utf8, format::utf16, I>` from it.  That would mean
adding a constructor that takes an input iterator and a `char32_t`.  This
would also mean that any user transcoding iterator types that are usable with
the `unpack_iterator_and_sentinel` CPO would also need to unpack their input
iterator into an iterator/code point pair, and that those user types would
also need to add this odd constructor.

This is all weird.  It's also a pretty small use case.  People don't use input
iterators that often.  Since this can always be added later, it is not being
proposed right now.

## Add a transcoding view and adaptors

```cpp
namespace std::uc {
  template<typename T>
  constexpr format @*format-of*@() {              // @*exposition only*@
    constexpr uint32_t size = sizeof(T);
    static_assert(is_integral<T>::value, "");
    static_assert(size == 1 || size == 2 || size == 4, "");
    constexpr format formats[] = {
      format::utf8,
      format::utf8,
      format::utf16,
      format::utf32,
      format::utf32};
    return formats[size];
  }

  template<class T>
    constexpr bool @*is-unpacking-owning-view*@ = false;
  template<class R, class V>
    constexpr bool @*is-unpacking-owning-view*@<unpacking_owning_view<R>> = true;

  template<ranges::range R>
    requires movable<R> && (!@*is-initializer-list*@<R>)
  class unpacking_owning_view : public ranges::view_interface<unpacking_owning_view<R>> {
    R r_ = R();

  public:
    constexpr unpacking_owning_view() requires default_initializable<R> = default;
    constexpr unpacking_owning_view(R&& r) : r_(std::move(r)) {}

    constexpr R& base() & noexcept { return r_; }
    constexpr const R& base() const & noexcept { return r_; }
    constexpr R&& base() && noexcept { return std::move(r_); }
    constexpr const R&& base() const && noexcept { return std::move(r_); }

    constexpr auto code_units() const noexcept {
      auto unpacked = uc::unpack_iterator_and_sentinel(ranges::begin(r_), ranges::end(r_));
      return ranges::subrange(unpacked.first, unpacked.last);
    }

    constexpr auto begin() const { return ranges::begin(r_); }
    constexpr auto end() const { return ranges::end(r_); }
  };

  template<format Format, utf_range V>
    requires ranges::view<V>
  class utf_view : public ranges::view_interface<utf_view<Format, V>> {
    V base_ = V();                                          // @*exposition only*@

    template<format FromFormat, class I, class S>
      static constexpr auto make_begin(I first, S last) {   // @*exposition only*@
        if constexpr (bidirectional_iterator<I>) {
          return utf_iterator<FromFormat, Format, I, S>{
            first, first, last};
        } else {
          return utf_iterator<FromFormat, Format, I, S>{first, last};
        }
      }
    template<format FromFormat, class I, class S>
      static constexpr auto make_end(I first, S last) {     // @*exposition only*@
        if constexpr (!same_as<I, S>) {
          return last;
        } else if constexpr (bidirectional_iterator<I>) {
          return utf_iterator<FromFormat, Format, I, S>{
            first, last, last};
        } else {
          return utf_iterator<FromFormat, Format, I, S>{last, last};
        }
      }

  public:
    constexpr utf_view() requires default_initializable<V> = default;
    constexpr utf_view(V base) : base_{std::move(base)} {}

    constexpr V base() const & requires copy_constructible<V> { return base_; }
    constexpr V base() && { return std::move(base_); }

    constexpr auto code_units() const noexcept
      requires copy_constructible<V> || @*is-unpacking-owning-view*@<V> {
      if constexpr (@*is-unpacking-owning-view*@<V>) {
        return base_.code_units();
      } else {
        return base_;
      }
    }

    constexpr auto begin() const {
      constexpr format from_format = @*format-of*@<ranges::range_value_t<V>>();
      if constexpr (@*is-unpacking-owning-view*@<V>) {
        return make_begin<from_format>(
          ranges::begin(base_.code_units()),
          ranges::end(base_.code_units()));
      } else {
        return make_begin<from_format>(
          ranges::begin(base_), ranges::end(base_));
      }
    }
    constexpr auto end() const {
      constexpr format from_format = @*format-of*@<ranges::range_value_t<V>>();
      if constexpr (@*is-unpacking-owning-view*@<V>) {
        return make_end<from_format>(
          ranges::begin(base_.code_units()),
          ranges::end(base_.code_units()));
      } else {
        return make_end<from_format>(
          ranges::begin(base_), ranges::end(base_));
      }
    }

    friend ostream & operator<<(ostream & os, utf_view v);
    friend wostream & operator<<(wostream & os, utf_view v);
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
    constexpr decltype(auto) unpack_range(R && r) {
      using T = remove_cvref_t<R>;
      if constexpr (ranges::forward_range<T>) {
        auto unpacked = uc::unpack_iterator_and_sentinel(ranges::begin(r), ranges::end(r));
        if constexpr (is_bounded_array_v<T>) {
          constexpr auto n = extent_v<T>;
          if (n && !r[n - 1])
            --unpacked.last;
        }
        if constexpr (ranges::borrowed_range<T> || is_lvalue_reference_v<R>) {
          return ranges::subrange(unpacked.first, unpacked.last);
        } else if constexpr (!same_as<decltype(unpacked.first), ranges::iterator_t<T>> ||
                             !same_as<decltype(unpacked.last), ranges::sentinel_t<T>>) {
          return unpacking_owning_view(std::move(r));
        } else {
          return forward<R>(r);
        }
      } else {
        return forward<R>(r);
      }
    }

  inline constexpr @*unspecified*@ as_utf8;
  inline constexpr @*unspecified*@ as_utf16;
  inline constexpr @*unspecified*@ as_utf32;
}
```

`unpacking_owning_view` comtains an owned range of type `R` and knows how to
unpack that range into code unit iterators using
`unpack_iterator_and_sentinel`.

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
`T` be `remove_cvref_t<decltype((E))>`.  Let `F` be the `format` enumerator
associated with `as_utfN`.  If `decltype((E))` does not model
`utf_range_like`, `as_utfN(E)` is ill-formed.  The expression `as_utfN(E)` is
expression-equivalent to:

- If `T` is a specialization of `empty_view` ([range.empty.view]), then
  `empty_view<@*format-to-type-t*@<F>()>{}`.

- Otherwise, if `T` is a specialization of `utfN_view`, then `V(E.base())`.

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

### Why there are three `utfN_view`s views plus `utf_view`

The views in `std::ranges` are constrained to accept only `std::ranges::view`
template parameters.  However, they accept `std::ranges::viewable_range`s in
practice, because they each have a deduction guide that likes like this:

```c++
template<class R>
utf8_view(R &&) -> utf8_view<views::all_t<R>>;
```

It's not possible to make this work for `utf_view`, since to use it you must
supply a `format` NTTP.  So, we need the `utfN_view`s.  It might be possible
to make `utf_view` an exposition-only implementation detail, but I think some
users might find use for it, especially in generic contexts.  For instance:

```c++
template<std::uc::format F, typename V>
auto f(std::uc::utf_view<F, V> const & view) {
    // Use F, V, and view here....
}
```

### `unpacking_owning_view`

To see why `unpacking_owning_view` is needed, consider:

```c++
struct my_text_type
{
    my_text_type() = default;
    my_text_type(std::u8string utf8) : utf8_(std::move(utf8)) {}

    auto begin() const { return utf_8_to_32_iterator(utf8_.begin(), utf8_.begin(), utf8_.end()); }
    auto end() const { return utf_8_to_32_iterator(utf8_.begin(), utf8_.end(), utf8_.end()); }

private:
    std::u8string utf8_;
};

void f() {
    auto view = my_text_type(u8"text") | std::uc::as_utf16;
}
```

This type `my_text_type` is a bit odd.  We cannot just unpack a `my_text_type`
rvalue and store the resulting unpacked range in a `utf_view`, because that
would create a view with dangling iterators.  We also cannot just store the
not-unpacked `my_text_type` either, because its iterator type is then not
unpacked, and `utf_view::begin()` would not work as written above.

We could just store `utf_view::base_` as it is given to us (that is, as a
`decltype(r)` when the uers writes `r | as_utfN`), then unpack it and form a
`utf_iterator` in each of `utf_view::begin()` and `utf_view::end()`.

Of course, we could do just that -- unpack in `utf_view::begin()` and
`utf_view::end()` before forming a `utf_iterator`.  The unpacking logic
currently exists in the `as_utfN` adaptors, and moving it into `utf_view`
seems to be at odds with how the existing standard adaptors work -- the
adaptors usually contain as much of the adaptation logic as possible, leaving
the view itself comparatively simple.  It also seems a shame to repeatedly
unpack in `utf_view::begin()` and `utf_view::end()` when, for the vast
majority of cases, just unpacking once in the adaptor would suffice.
(Remember, we only need to make this special case of an rvalue with unpackable
iterators work, like `my_text_type(u8"text") | std::uc::as_utf16`.)

Another option would be to give `utf_view` and all the `utfN_view`s knowledge
of the special case, in the form of a `bool` template parameter indicating
that the adapted view needs to be unpacked.  Adding an NTTP like that to
`utfN_view` would make it awkward to use outside the context of the `as_utfN`
adaptors.  For instance:

```c++
template<typename V>
void f(std::uc::utf32_view<V, /* ??? */> const & utf32) {
}
```

What do we write for the `/* ??? */`, the NTTP that indicates whether `V` is
already unpacked, or we need to unpack `V` it?  We have to do a nontrivial
amount of work involving `V` to know what to write there.

### More examples

```c++
static_assert(std::is_same_v<
              decltype(my_text_type(u8"text") | std::uc::as_utf16),
              std::uc::utf16_view<std::uc::unpacking_owning_view<
                  my_text_type,
                  std::ranges::subrange<std::u8string::const_iterator>>>>);

static_assert(std::is_same_v<
              decltype(u8"text" | std::uc::as_utf16),
              std::uc::utf16_view<std::ranges::subrange<const char8_t *>>>);

static_assert(std::is_same_v<
              decltype(std::u8string(u8"text") | std::uc::as_utf16),
              std::uc::utf16_view<std::ranges::owning_view<std::u8string>>>);

std::u8string str = u8"text";

static_assert(std::is_same_v<
              decltype(str | std::uc::as_utf16),
              std::uc::utf16_view<
                  std::ranges::subrange<std::u8string::iterator>>>);

static_assert(std::is_same_v<
              decltype(str.c_str() | std::uc::as_utf16),
              std::uc::utf16_view<std::ranges::subrange<
                  const char8_t *,
                  std::uc::null_sentinel_t>>>);

static_assert(std::is_same_v<
              decltype(std::ranges::empty_view<int>{} | std::uc::as_utf16),
              std::ranges::empty_view<char16_t>>);

std::u16string str2 = u"text";

static_assert(std::is_same_v<
              decltype(str2 | std::uc::as_utf16),
              std::uc::utf16_view<
                  std::ranges::subrange<std::u16string::iterator>>>);

static_assert(std::is_same_v<
              decltype(str2.c_str() | std::uc::as_utf16),
              std::uc::utf16_view<std::ranges::subrange<
                  const char16_t *,
                  std::uc::null_sentinel_t>>>);
```

### Why `utf_view` always uses `utf_iterator`, even in UTF-N to UTF-N cases

You might expect that if `r` in `r | as_utfN` is already in UTF-N, `r |
as_utfN` might just be `r`.  This is not what the `as_utfN` adaptors do,
though.

The adaptors each produce a view `utfv` that stores a view of type `V`, where
`V` is made from the result of unpacking `r`.  Further, `utfv.begin()` is
always a specialization of `utf_iterator`.  `utfv.end()` is also a
specialization of `utf_iterator` (if `common_range<V>`), or otherwise the
sentinel value for `V`.

This gives `r | as_utfN` some nice properties that are consistent.  With the
exception of `empty_view<T>{} | as_utfN`, the following are always true:

- `r | as_utfN` produces well-formed UTF.  Since the default `ErrorHandler`
  template parameter to `utf_iterator` `use_replacement_character` is always
  used, any ill-formed UTF is replaced with `replacement_character`.  This is
  true even when the input was already UTF-N.  Remember, the input could have
  been UTF-N but had ill-formed UTF in it.

- `r | as_utfN` has a consistent API.  If `r | as_utfN` were sometimes `r`,
  and since `r` may be a reference to an array, you'd have to use
  `std::ranges::begin(r)` and `::end(r)` all the time.  However, you'd
  probably write `r.begin()` and `r.end()`, only to one day get bitten by an
  array-reference `r`.

- `r | as_utfN` is formattable/printable.  This means you can adapt anything
  that can be UTF-transcoded to do I/O in a consistent way.  For example:

```c++
    auto str0 = std::format("{}", std::u8string{});                    // Error: ill-formed!
    auto str1 = std::format("{}", std::u8string{} | std::uc::as_utf8); // Ok.
```

### Add `utf_view` specialization of `formatter`

These should be added to the list of "the debug-enabled string type
specializations" in [format.formatter.spec].  This allows `utf_view` and
`utfN_view` to be used in `std::format()` and `std::print()`.  The intention
is that the formatter will transcode to UTF-8 if the formatter's `CharT` is
`char`, or to UTF-16 or UTF-32 (which one is implementation defined) if the
formatter's `CharT` is `wchar_t` -- if transcoding is necessary at all.

```cpp
namespace std {
  template<uc::format Format, class V, class CharT>
  struct formatter<uc::utf_view<Format, V>, CharT> {
  private:
    formatter<basic_string<CharT>, CharT> @*underlying_*@;                // @*exposition only*@

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

```c++
template<class FormatContext>
  typename FormatContext::iterator
    format(const uc::utf_view<Format, V>& view, FormatContext& ctx) const;
```

Effects: Equivalent to:
```c++
auto adaptor = @*see below*@;
return @*underlying_*@.format(basic_string<CharT>(from_range, view | adaptor), ctx);
```

`adaptor` is `uc::as_utf8` if `CharT` is `char`.  Otherwise, it is
implementation defined whether `adaptor` is `uc::as_utf16` or `uc::as_utf32`.

```c++
constexpr void set_debug_format() noexcept;
```

Effects: Equivalent to:
```c++
@*underlying_*@.set_debug_format();
```

## Add a feature test macro

Add the feature test macro `__cpp_lib_unicode_transcoding`.

## Design notes

None of the proposed interfaces is subject to change in future versions of
Unicode; each relates to the guaranteed-stable subset.  Just sayin'.

None of the proposed interfaces allocates or throws, unless the user supplies
a throwing `ErrorHandler` template parameter to `utf_iterator`.

The proposed interfaces allow users to choose amongst multiple
convenience-vs-compatibility tradeoffs.  Explicitly, they are:

- If you need compatibility with existing iterator-based algorithms (such as
  the standard algorithms), use the transcoding iterators.
- If you want streamability or the convenience of constructing ranges with a
  single `| as_utfN` adaptor use, use the transcoding views.

All the transcoding iterators allow you access to the underlying iterator via
`.base()` (except when adapting an input iterator), following the convention
of the iterator adaptors already in the standard.

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
