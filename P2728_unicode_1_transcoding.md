---
title: "Unicode in the Library, Part 1: UTF Transcoding"
document: P2728R1
date: 2022-11-20
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
- Pointers are important.  The fastest transcoding will always be done in
  terms of pointers (due to the speed of SIMD operations in particular).  We
  should support all-pointer-interface transcoding.
- Transcoding cannot be a black box; sometimes you need to be able to find
  where there is a break in the encoding, or to detect whether a sequence has
  any broken encodings in it.  We should provide utility functions that let
  users investigate these states.
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
template<class UTF16Range>
void process_input(UTF16Range && r);

std::string input = get_utf8_input(); // A std::string used as a UTF-8 string.

process_input(input | std::uc::as_utf16);
```

## Case 2: Adapt to an existing iterator interface taking a different UTF

This time, we have a generic iterator interface we want to transcode into, so
we want to use the transcoding iterators.

```cpp
// A generic function that accepts sequences of UTF-16.
template<class UTF16Iter>
void process_input(UTF16Iter first, UTF16Iter last);

std::string input = get_utf8_input(); // A std::string used as a UTF-8 string.

process_input(std::uc::utf_8_to_16_iterator(input.begin(), input.begin(), input.end()),
              std::uc::utf_8_to_16_iterator(input.begin(), input.end(), input.end()));

// Even more conveniently:
auto const utf16_view = input | std::uc::as_utf16;
process_input(utf16_view.begin(), utf16.end());
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

  template<typename T>
    concept transcoding_error_handler =
      requires(T t, char const * msg) { { t(msg) } -> code_point; };

}
```

## Add a standard null-terminated sequence sentinel

```cpp
namespace std {
  struct null_sentinel_t {
    constexpr null_sentinel_t base() const { return {}; }

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

## Add constants and utility functions that query the state of UTF sequences (well-formedness, etc.)

```cpp
namespace std::uc {
  inline constexpr char16_t high_surrogate_base = 0xd7c0;
  inline constexpr char16_t low_surrogate_base = 0xdc00;
  inline constexpr char32_t high_surrogate_min = 0xd800;
  inline constexpr char32_t high_surrogate_max = 0xdbff;
  inline constexpr char32_t low_surrogate_min = 0xdc00;
  inline constexpr char32_t low_surrogate_max = 0xdfff;
  inline constexpr char32_t replacement_character = 0xfffd;

  // Returns is_high_surrogate(c) || is_low_surrogate(c).
  constexpr bool is_surrogate(char32_t c);

  // Returns true iff c is a Unicode high surrogate.
  constexpr bool is_high_surrogate(char32_t c);

  // Returns true iff c is a Unicode low surrogate.
  constexpr bool is_low_surrogate(char32_t c);

  // Returns true iff c is a Unicode reserved noncharacter.
  constexpr bool is_reserved_noncharacter(char32_t c);

  // Returns true iff c is a valid Unicode scalar value.
  constexpr bool is_scalar_value(char32_t c);

  // Returns true iff c is a Unicode scalar value not in the reserved
  // range.
  constexpr bool is_unreserved_scalar_value(char32_t c);

  // Returns true iff c is a UTF-8 lead code unit (which must be followed
  // by 1-3 following units).
  constexpr bool is_lead_code_unit(char8_t c);

  // Returns true iff c is a UTF-8 continuation (non-lead) code unit.
  constexpr bool is_continuation(char8_t c);

  // Given the first (and possibly only) code unit of a UTF-8-encoded code
  // point, returns the number of bytes occupied by that code point (in the
  // range [1, 4]).  Returns a value < 0 if first_unit is not a valid
  // initial UTF-8 code unit.
  constexpr int utf8_code_units(char8_t first_unit);

  // Given the first (and possibly only) code unit of a UTF-16-encoded code
  // point, returns the number of code units occupied by that code point
  // (in the range [1, 2]).  Returns a value < 0 if first_unit is
  // not a valid initial UTF-16 code unit.
  constexpr int utf16_code_units(char16_t first_unit);

  // Returns the first code unit in [ranges::begin(r), ranges::end(r)) that
  // is not properly UTF-8 encoded, or ranges::begin(r) + ranges::distance(r) if
  // no such code unit is found.
  template<utf8_range R>
    requires ranges::forward_range<R>
      constexpr ranges::borrowed_iterator_t<R> find_invalid_encoding(R && r);

  // Returns the first code unit in [ranges::begin(r), ranges::end(r)) that
  // is not properly UTF-16 encoded, or ranges::begin(r) + ranges::distance(r) if
  // no such code unit is found.
  template<utf16_range R>
    requires ranges::forward_range<R>
      constexpr ranges::borrowed_iterator_t<R> find_invalid_encoding(R && r);

  // Returns true iff r is properly UTF-8 encoded.
  template<utf8_range R>
    requires ranges::forward_range<R>
      constexpr bool encoded(R && r);

  // Returns true iff r is properly UTF-16 encoded.
  template<utf16_range R>
    requires ranges::forward_range<R>
      constexpr bool encoded(R && r);

  // Returns true iff r is empty or the initial UTF-8 code units in r form a valid
  // Unicode code point.
  template<utf8_range R>
    requires ranges::forward_range<R>
      constexpr bool starts_encoded(R && r);

  // Returns true iff r is empty or the initial UTF-16 code units in r form a valid
  // Unicode code point.
  template<utf16_range R>
    requires ranges::forward_range<R>
      constexpr bool starts_encoded(R && r);

  // Returns true iff r is empty or the final UTF-8 code units in r form a valid
  // Unicode code point.
  template<utf8_range R>
    requires ranges::bidirectional_range<R> && ranges::common_range<R>
      constexpr bool ends_encoded(R && r);

  // Returns true iff r is empty or the final UTF-16 code units in r form a valid
  // Unicode code point.
  template<utf16_range R>
    requires ranges::bidirectional_range<R> && ranges::common_range<R>
      constexpr bool ends_encoded(R && r);
}
```

## Add the transcoding iterators

I'm using [P2727](https://isocpp.org/files/papers/P2727R0.html)'s
`iterator_interface` here for simplicity.

```cpp
namespace std::uc {
  // An error handler type that can be used with the converting iterators;
  // provides the Unicode replacement character on errors.
  struct use_replacement_character {
    constexpr char32_t operator()(const char*) const { return replacement_character; }
  };
  
  template<typename I>
  auto @*bidirectional-at-most*@() {  // @*exposition only*@
    if constexpr (bidirectional_iterator<I>) {
      return bidirectional_iterator_tag{};
    } else if constexpr (forward_iterator<I>) {
      return forward_iterator_tag{};
    } else if constexpr (input_iterator<I>) {
      return input_iterator_tag{};
    }
  }
  
  template<typename I>
  using @*bidirectional-at-most-t*@ = decltype(@*bidirectional-at-most*@<I>()); // @*exposition only*@

  template<
    utf32_iter I,
    sentinel_for<I> S = I,
    transcoding_error_handler ErrorHandler = use_replacement_character>
  struct utf_32_to_8_iterator
    : iterator_interface<utf_32_to_8_iterator<I, S, ErrorHandler>, @*bidirectional-at-most-t*@<I>, char8_t, char8_t> {
    constexpr utf_32_to_8_iterator();
    explicit constexpr utf_32_to_8_iterator(I first, I it, S last);
    template<class I2, class S2>
      requires convertible_to<I2, I> && convertible_to<S2, S>
        constexpr utf_32_to_8_iterator(
          const utf_32_to_8_iterator<I2, S2, ErrorHandler>& other);

    constexpr I begin() const { return first_; }
    constexpr S end() const { return last_; }

    constexpr char8_t operator*() const { return buf_[index_]; }

    constexpr I base() const { return it_; }

    constexpr utf_32_to_8_iterator& operator++();
    constexpr utf_32_to_8_iterator& operator--();

    template<class I1, class S1, class I2, class S2, class ErrorHandler2>
    friend constexpr bool operator==(
      const utf_32_to_8_iterator<I1, S1, ErrorHandler2>& lhs,
      const utf_32_to_8_iterator<I2, S2, ErrorHandler2>& rhs)
        requires requires { lhs.base() == rhs.base(); }
          { return lhs.base() == rhs.base() && lhs.index_ == rhs.index_; }

    friend constexpr bool operator==(utf_32_to_8_iterator lhs, utf_32_to_8_iterator rhs)
      { return lhs.base() == rhs.base() && lhs.index_ == rhs.index_; }

    using @*base-type*@ =         // @*exposition only*@
      iterator_interface<utf_32_to_8_iterator<I, S, ErrorHandler>,
                         @*bidirectional-at-most-t*@<I>,
                         char32_t,
                         char32_t>;
    using @*base-type*@::operator++;
    using @*base-type*@::operator--;

  private:
    I first_;                 // @*exposition only*@
    I it_;                    // @*exposition only*@
    S last_;                  // @*exposition only*@
    int index_;               // @*exposition only*@
    array<char8_t, 5> buf_;   // @*exposition only*@

    template<utf32_iter I2, sentinel_for<I2> S2, transcoding_error_handler ErrorHandler2>
    friend struct utf_32_to_8_iterator;
  };

  template<class I, class S, class ErrorHandler>
    constexpr bool operator==(
      utf_32_to_8_iterator<I, S, ErrorHandler> lhs, S rhs)
        requires requires { lhs.base() == rhs; };

  template<
    utf8_iter I,
    sentinel_for<I> S = I,
    transcoding_error_handler ErrorHandler = use_replacement_character>
  struct utf_8_to_32_iterator
    : iterator_interface<utf_8_to_32_iterator<I, S, ErrorHandler>, @*bidirectional-at-most-t*@<I>, char32_t, char32_t> {
    constexpr utf_8_to_32_iterator();
    explicit constexpr utf_8_to_32_iterator(I first, I it, S last);
    template<class I2, class S2>
      requires convertible_to<I2, I> && convertible_to<S2, S>
        constexpr utf_8_to_32_iterator(
          const utf_8_to_32_iterator<I2, S2, ErrorHandler>& other);

    constexpr I begin() const { return first_; }
    constexpr S end() const { return last_; }

    constexpr char32_t operator*() const;

    constexpr I base() const { return it_; }

    constexpr utf_8_to_32_iterator& operator++();
    constexpr utf_8_to_32_iterator& operator--();

    friend constexpr bool operator==(utf_8_to_32_iterator lhs, utf_8_to_32_iterator rhs)
      { return lhs.base() == rhs.base(); }

    using @*base-type*@ =         // @*exposition only*@
      iterator_interface<utf_8_to_32_iterator<I, S, ErrorHandler>,
                         @*bidirectional-at-most-t*@<I>,
                         char32_t,
                         char32_t>;
    using @*base-type*@::operator++;
    using @*base-type*@::operator--;

  private:
    I first_;                 // @*exposition only*@
    I it_;                    // @*exposition only*@
    S last_;                  // @*exposition only*@

    template<utf8_iter I2, sentinel_for<I2> S2, transcoding_error_handler ErrorHandler2>
    friend struct utf_8_to_16_iterator;

    template<utf8_iter I2, sentinel_for<I2> S2, transcoding_error_handler ErrorHandler2>
    friend struct utf_8_to_32_iterator;
  };

  template<class I, class S, class ErrorHandler>
  constexpr bool operator==(
    const utf_8_to_32_iterator<I, S, ErrorHandler>& lhs, Sentinel rhs)
      requires requires { lhs.base() == rhs; };

  template<class I1, class S1, class I2, class S2, class ErrorHandler>
  constexpr bool operator==(
    const utf_8_to_32_iterator<I1, S1, ErrorHandler>& lhs,
    const utf_8_to_32_iterator<I2, S2, ErrorHandler>& rhs)
      requires requires { lhs.base() == rhs.base(); };

  template<
    utf32_iter I,
    sentinel_for<I> S = I,
    transcoding_error_handler ErrorHandler = use_replacement_character>
  struct utf_32_to_16_iterator
    : iterator_interface<utf_32_to_16_iterator<I, S, ErrorHandler>, @*bidirectional-at-most-t*@<I>, char16_t, char16_t> {
    constexpr utf_32_to_16_iterator();
    explicit constexpr utf_32_to_16_iterator(I first, I it, S last);
    template<class I2, class S2>
      requires convertible_to<I2, I> && convertible_to<S2, S>
        constexpr utf_32_to_16_iterator(
          const utf_32_to_16_iterator<I2, S2, ErrorHandler>& other);

    constexpr I begin() const { return first_; }
    constexpr S end() const { return last_; }

    constexpr char16_t operator*() const
    { return buf_[index_]; }

    constexpr I base() const { return it_; }

    constexpr utf_32_to_16_iterator& operator++();
    constexpr utf_32_to_16_iterator& operator--();

    template<class I1, class S1, class I2, class S2, class ErrorHandler2>
    friend constexpr bool operator==(
      const utf_32_to_16_iterator<I1, S1, ErrorHandler2>& lhs,
      const utf_32_to_16_iterator<I2, S2, ErrorHandler2>& rhs)
        requires requires { lhs.base() == rhs.base(); }
          { return lhs.base() == rhs.base() && lhs.index_ == rhs.index_; }

    friend constexpr bool operator==(utf_32_to_16_iterator lhs, utf_32_to_16_iterator rhs)
      { return lhs.base() == rhs.base() && lhs.index_ == rhs.index_; }

    using @*base-type*@ =         // @*exposition only*@
      iterator_interface<utf_32_to_16_iterator<I, S, ErrorHandler>,
                         @*bidirectional-at-most-t*@<I>,
                         char16_t,
                         char16_t>;
    using @*base-type*@::operator++;
    using @*base-type*@::operator--;

  private:
    I first_;                 // @*exposition only*@
    I it_;                    // @*exposition only*@
    S last_;                  // @*exposition only*@
    int index_;               // @*exposition only*@
    array<char16_t, 4> buf_;  // @*exposition only*@

    template<utf32_iter I2, sentinel_for<I2> S2, transcoding_error_handler ErrorHandler2>
    friend struct utf_32_to_16_iterator;
  };

  template<class I, class S, class ErrorHandler>
  constexpr bool operator==(
    const utf_32_to_16_iterator<I, S, ErrorHandler>& lhs, Sentinel rhs)
      requires requires { lhs.base() == rhs; };

  template<
    utf16_iter I,
    sentinel_for<I> S = I,
    transcoding_error_handler ErrorHandler = use_replacement_character>
  struct utf_16_to_32_iterator
    : iterator_interface<utf_16_to_32_iterator<I, S, ErrorHandler>, @*bidirectional-at-most-t*@<I>, char32_t, char32_t> {
    constexpr utf_16_to_32_iterator();
    explicit constexpr utf_16_to_32_iterator(I first, I it, S last);
    template<class I2, class S2>
      requires convertible_to<I2, I> && convertible_to<S2, S>
        constexpr utf_16_to_32_iterator(
          const utf_16_to_32_iterator<I2, S2, ErrorHandler>& other);

    constexpr I begin() const { return first_; }
    constexpr S end() const { return last_; }

    constexpr char32_t operator*() const;

    constexpr I base() const { return it_; }

    constexpr utf_16_to_32_iterator& operator++();
    constexpr utf_16_to_32_iterator& operator--();

    friend constexpr bool operator==(utf_16_to_32_iterator lhs, utf_16_to_32_iterator rhs)
      { return lhs.base() == rhs.base(); }

    using @*base-type*@ =         // @*exposition only*@
      iterator_interface<utf_16_to_32_iterator<I, S, ErrorHandler>,
                         @*bidirectional-at-most-t*@<I>,
                         char32_t,
                         char32_t>;
    using @*base-type*@::operator++;
    using @*base-type*@::operator--;

  private:
    I first_;                 // @*exposition only*@
    I it_;                    // @*exposition only*@
    S last_;                  // @*exposition only*@

    template<utf32_iter I2, sentinel_for<I2> S2, transcoding_error_handler ErrorHandler2>
    friend struct utf_32_to_16_iterator;

    template<utf16_iter I2, sentinel_for<I2> S2, transcoding_error_handler ErrorHandler2>
    friend struct utf_16_to_32_iterator;
  };

  template<class I, class S, class ErrorHandler>
  constexpr bool operator==(
    const utf_16_to_32_iterator<I, S, ErrorHandler>& lhs, Sentinel rhs)
      requires requires { lhs.base() == rhs; };

  template<
    class I1, class S1,
    class I2, class S2,
    class ErrorHandler>
  constexpr bool operator==(
    const utf_16_to_32_iterator<I1, S1, ErrorHandler>& lhs,
    const utf_16_to_32_iterator<I2, S2, ErrorHandler>& rhs)
      requires requires { lhs.base() == rhs.base(); };

  template<
      utf16_iter I,
      sentinel_for<I> S = I,
      transcoding_error_handler ErrorHandler = use_replacement_character>
  struct utf_16_to_8_iterator
    : iterator_interface<utf_16_to_8_iterator<I, S, ErrorHandler>, @*bidirectional-at-most-t*@<I>, char8_t, char8_t> {
    constexpr utf_16_to_8_iterator();
    explicit constexpr utf_16_to_8_iterator(I first, I it, S last);
    template<class I2, class S2>
      requires convertible_to<I2, I> && convertible_to<S2, S>
        constexpr utf_16_to_8_iterator(const utf_16_to_8_iterator<I2, S2>& other);

    constexpr I begin() const { return first_; }
    constexpr S end() const { return last_; }

    constexpr char8_t operator*() const { return buf_[index_]; }

    constexpr I base() const { return it_; }

    constexpr utf_16_to_8_iterator& operator++();
    constexpr utf_16_to_8_iterator& operator--();

    template<class I1, class S1, class I2, class S2, class ErrorHandler2>
    friend constexpr bool operator==(
      const utf_16_to_8_iterator<I1, S1, ErrorHandler2>& lhs,
      const utf_16_to_8_iterator<I2, S2, ErrorHandler2>& rhs)
        requires requires { lhs.base() == rhs.base(); }
          { return lhs.base() == rhs.base() && lhs.index_ == rhs.index_; }

    friend constexpr bool operator==(utf_16_to_8_iterator lhs, utf_16_to_8_iterator rhs)
      { return lhs.base() == rhs.base() && lhs.index_ == rhs.index_; }

    using @*base-type*@ =         // @*exposition only*@
      iterator_interface<utf_16_to_8_iterator<I, S, ErrorHandler>,
                         @*bidirectional-at-most-t*@<I>,
                         char8_t,
                         char8_t>;
    using @*base-type*@::operator++;
    using @*base-type*@::operator--;

  private:
    I first_;                 // @*exposition only*@
    I it_;                    // @*exposition only*@
    S last_;                  // @*exposition only*@
    int index_;               // @*exposition only*@
    array<char8_t, 5> buf_;   // @*exposition only*@

    template<utf16_iter I2, sentinel_for<I2> S2, transcoding_error_handler ErrorHandler2>
    friend struct utf_16_to_8_iterator;
  };

  template<class I, class S, class ErrorHandler>
  constexpr bool operator==(
    const utf_16_to_8_iterator<I, S, ErrorHandler>& lhs, Sentinel rhs)
      requires requires { lhs.base() == rhs; };

  template<class I1, class S1, class I2, class S2, class ErrorHandler>
  constexpr bool operator==(
    const utf_16_to_8_iterator<I1, S1, ErrorHandler>& lhs,
    const utf_16_to_8_iterator<I2, S2, ErrorHandler>& rhs)
      requires requires { lhs.base() == rhs.base(); };

  template<
    utf8_iter I,
    sentinel_for<I> S = I,
    transcoding_error_handler ErrorHandler = use_replacement_character>
  struct utf_8_to_16_iterator
    : iterator_interface<utf_8_to_16_iterator<I, S, ErrorHandler>, @*bidirectional-at-most-t*@<I>, char16_t, char16_t> {
    constexpr utf_8_to_16_iterator();
    explicit constexpr utf_8_to_16_iterator(I first, I it, S last);
    template<class I2, class S2>
      requires convertible_to<I2, I> && convertible_to<S2, S>
        constexpr utf_8_to_16_iterator(
          const utf_8_to_16_iterator<I2, S2, ErrorHandler>& other);

    constexpr I begin() const { return it_.begin(); }
    constexpr S end() const { return it_.end(); }

    constexpr char16_t operator*() const { return buf_[index_]; }

    constexpr I base() const { return it_.base(); }

    constexpr utf_8_to_16_iterator& operator++();
    constexpr utf_8_to_16_iterator& operator--();

    template<class I1, class S1, class I2, class S2, class ErrorHandler2>
    friend constexpr bool operator==(
      const utf_8_to_16_iterator<I1, S1, ErrorHandler2>& lhs,
      const utf_8_to_16_iterator<I2, S2, ErrorHandler2>& rhs)
        requires requires { lhs.base() == rhs.base()' }
          { return lhs.base() == rhs.base() && lhs.index_ == rhs.index_; }

    friend constexpr bool operator==(utf_8_to_16_iterator lhs, utf_8_to_16_iterator rhs)
      { return lhs.base() == rhs.base() && lhs.index_ == rhs.index_; }

    using @*base-type*@ =                // @*exposition only*@
      iterator_interface<utf_8_to_16_iterator<I, S, ErrorHandler>,
                         @*bidirectional-at-most-t*@<I>,
                         char16_t,
                         char16_t>;
    using @*base-type*@::operator++;
    using @*base-type*@::operator--;

  private:
    utf_8_to_32_iterator<I, S> it_;  // @*exposition only*@
    int index_;                      // @*exposition only*@
    array<char16_t, 4> buf_;         // @*exposition only*@

    template<utf8_iter I2, sentinel_for<I2> S2, transcoding_error_handler ErrorHandler2>
    friend struct utf_8_to_16_iterator;
  };

  template<class I, class S, class ErrorHandler>
    constexpr bool operator==(
      const utf_8_to_16_iterator<I, S, ErrorHandler>& lhs, Sentinel rhs)
        requires requires { lhs.base() == rhs; };
}
```

## Add a transcoding view

### Add the view proper

```cpp
namespace std::uc {
  template<typename V>
    using @*utf-view-iter-t*@ = @*see below*@;                    // @*exposition only*@
  template<typename V>
    using @*utf-view-sent-t*@ = @*see below*@;                    // @*exposition only*@
  template<format Format, typename Unpacked>
    constexpr auto @*make-utf-view-iter*@(Unpacked unpacked); // @*exposition only*@
  template<format Format, typename Unpacked>
    constexpr auto @*make-utf-view-sent*@(Unpacked unpacked); // @*exposition only*@

  template<format Format, utf_range_like V>
    requires ranges::view<V> || utf_pointer<V>
  struct utf_view : stl_interfaces::view_interface<utf_view<Format, V>>
  {
    using from_iterator = @*utf-view-iter-t*@<V>;
    using from_sentinel = @*utf-view-sent-t*@<V>;

    using iterator = decltype(@*make-utf-view-iter*@<Format>(
      uc::unpack_iterator_and_sentinel(declval<from_iterator>(), declval<from_sentinel>())));
    using sentinel = decltype(@*make-utf-view-iter*@<Format>(
      uc::unpack_iterator_and_sentinel(declval<from_iterator>(), declval<from_sentinel>())));

    constexpr utf_view() {}
    constexpr utf_view(V base);

    constexpr iterator begin() const { return first_; }
    constexpr sentinel end() const { return last_; }

    friend ostream& operator<<(ostream& os, utf_view v);
    friend wostream& operator<<(wostream& os, utf_view v);

  private:
    iterator first_;
    [[no_unique_address]] sentinel last_;
  };
}

namespace std::ranges {
  template<uc::format Format, typename V>
    inline constexpr bool enable_borrowed_range<uc::utf_view<Format, V>> = true;
}
```

`@*utf-view-iter-t*@` evaluates to `V` if `V` is a pointer, and
`decltype(std::ranges::begin(std::declval<V>()))` otherwise.
`@*utf-view-sent-t*@` evaluates to `null_sentinel_t` if `V` is a pointer, and
`decltype(std::ranges::end(std::declval<V>()))` otherwise.

`@*make-utf-view-iter*@` makes a transcoding iterator that produces the UTF
format `format` from the result of a call to
`std::uc::unpack_iterator_and_sentinel()`, and similarly
`@*make-utf-view-sent*@` makes a sentinel from the result of a call to
`std::uc::unpack_iterator_and_sentinel()`.

The `ostream` and `wostream` stream operators transcode the `utf_view` to
UTF-8 and UTF-16 respectively (if transcoding is needed), and the `wostream`
overload is only defined on Windows.

### Add `as_utfN` view adaptors

Each `as_utfN` view adaptor adapts a `utf_range_like` (meaning an range or a
null-terminated pointer), and returns a `utfN_view` that may do transcoding
(if the inputs are not UTF-N) or may not do transcoding (if the inputs are
UTF-N).

```cpp
namespace std::uc {
  inline @*unspecified*@ as_utf8;
  inline @*unspecified*@ as_utf16;
  inline @*unspecified*@ as_utf32;
}
```

Each of these `as_utfN` adaptors produces a `uf_view<format::utfN, ...>`.
Here is some psuedo-wording for that hopefully clarifies.

Let `E` be an expression, and let `T` be `remove_cvref_t<decltype((E))>`.  The
expression `as_utfN(E)` is expression-equivalent to:

- If `T` is a specialization of `empty_view` ([range.empty.view]), then `decay-copy(E)`.

- Otherwise, if `ranges::iterator_t<T>` models `code_unit_iter<format::utfN>`,
  then `decay-copy(E)`.

- Otherwise, `utf_view<format::utfN, T>(E)`.


### Add `utfN_view` specializations of `formatter`

These should be added to the list of "the debug-enabled string type
specializations" in [format.formatter.spec].  This allows all three kinds of
UTF views to be used in `std::format()` and `std::print()`.  The intention is
that each one will transcode to UTF-8 if the formatter's `charT` is `char`, or
to UTF-16 if the formatter's `charT` is `wchar_t` -- if transcoding is
necessary at all.

```cpp
template<class I, class S>
  struct formatter<uc::utf8_view<I, S>, charT>;
template<class I, class S>
  struct formatter<uc::utf16_view<I, S>, charT>;
template<class I, class S>
  struct formatter<uc::utf32_view<I, S>, charT>;
```

### Add `unpack_iterator_and_sentinel` CPO for iterator "unpacking"

```cpp
struct no_op_repacker {
  template<typename T>
    T operator()(T x) const { return x; }
};

template<typename RepackedIterator, typename I, typename S, typename Then>
struct repacker {
  auto operator()(I it) const { return then(RepackedIterator(first, it, last)); }

  I first;
  [[no_unique_address]] S last;
  [[no_unique_address]] Then then;
};

template<format FormatTag, utf_iter I, sentinel_for<I> S, typename Repack>
struct utf_tagged_range {
  static constexpr format format_tag = FormatTag;

  I first;
  [[no_unique_address]] S last;
  [[no_unique_address]] Repack repack;
};

// CPO equivalent to:
template<utf_iter I, sentinel_for<I> S, typename Repack = no_op_repacker>
constexpr auto unpack_iterator_and_sentinel(I first, S last, Repack repack = Repack());
```

A simple way to represent a transcoding view is as a pair of transcoding
iterators. However, there is a problem with that approach, since a
`utf32_view<utf_8_to_32_iterator<char const *>>` would be a range the size of
6 pointers. Worse yet, a
`utf32_view<utf_8_to_16_iterator<utf_16_to_32_iterator<char const *>>>` would
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
static_assert(std::is_same<decltype(range),
                           std::uc::utf8_view<std::string::iterator>>::value, "");
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
  requires(utf8_code_unit<iter_value_t<I>> || utf16_code_unit<iter_value_t<I>>)
transcode_result<I, O> transcode_to_utf32(I first, S last, O out);
```

For such interfaces, it can be difficult in the general case to form an
iterator of type `I` to return to the user:

```c++
template<input_iterator I, sentinel_for<I> S, output_iterator<char8_t> O>
    requires(utf8_code_unit<iter_value_t<I>> || utf16_code_unit<iter_value_t<I>>)
transcode_result<I, O> transcode_to_utf32(I first, S last, O out) {
    // Get the input as UTF-32.
    auto r = uc::utf32_view(first, last);

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
    requires(utf8_code_unit<std::iter_value_t<I>> || utf16_code_unit<std::iter_value_t<I>>)
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
