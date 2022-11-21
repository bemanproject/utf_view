---
title: "Unicode in the Library, Part 1: UTF Transcoding"
document: P2728R0
date: 2022-11-20
audience:
  - LEWG-I
  - LEWG
author:
  - name: Zach Laine
    email: <whatwasthataddress@gmail.com>
toc: false
monofont: "DejaVu Sans Mono"

---

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
  transcoding.  This includes support for sentinels.
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
  
# The shortest Unicode primer imaginable

There are multiple encoding types defined in Unicode: UTF-8, UTF-16, and
UTF-32.

A *code unit* is the lowest-level datum-type in your Unicode data. Examples
are a `char` in UTF-8 and a `uint32_t` in UTF-32.

A *code point* is a 32-bit integral value that represents a single Unicode
value. Examples are U+0041 "A" "LATIN CAPITAL LETTER A" and U+0308 "¨"
"COMBINING DIAERESIS".

A code point may be consist of multiple code units.  For instance, 3 UTF-8
code units in sequence may encode a particular code unit.

# Use cases

There are several contexts to consider here:

- Coding convenience, terseness, and clarity.
- Performance.
- Compatability with other APIs.

Some times the contexts are in alignment, and sometimes they are in conflict.
We need multiple ways to do transcoding to cover all of those contexts.  Let's
make things more concrete:

## Case 1: Trascode a buffer as fast as possible

We care primarily about performance in this use case, so everything is a
pointer.  Also, our wire-communications layer knows nothing about the UTFs, so
we need to use some of the utility functions to make sure we don't process
partially-received UTF-8 sequences.

```cpp
// Using same size to ensure the transcode operation always has room.
char utf8_buf[buf_size];
char utf16_buf[buf_size];

char * read_first = utf8_buf;
while (true) {
    // Reads off a wire; may contain partial UTF-8 sequences at the ends of
    // some reads.
    char * buf_last = read_into_utf8_buffer(read_first, utf8_buf + buf_size);
    
    if (buf_last == read_first)
        continue;

    // find the last whole UTF-8 sequence, so we don't feed partial sequences
    // to the algorithm below.
    char * last = buf_last;
    auto const last_lead = std::ranges::find_last_if(
        utf8_buf, buf_last, std::uc::lead_code_unit);
    if (!last_lead.empty()) {
        auto const dist_from_end = buf_last - last_lead.begin();
        assert(dist_from_end <= 4);
        if (std::uc::utf8_code_units(*last_lead.begin()) != dist_from_end)
            last = last_lead.begin();
    }

    // Same interface as std::ranges::copy(), except that it converts as it copies.
    auto const result = std::uc::transcode_to_utf16(utf8_buf, last, utf16_buf);
    
    // Do something with the resulting UTF-16 buffer contents.
    send_utf16_somewhere(utf16_buf, result.out);

    // Copy partial UTF-8 sequence to start of buffer.
    read_first = std::ranges::copy_backward(last, buf_last, utf8_buf).out;
}
```

## Case 2: Transcode an object as fast as possible

`my_string` does not provide a pointer-based interface, so we need to use more
generic facilities for this case.

```cpp
struct my_string; // Some string type with *non-pointer* iterators.

my_string input = get_utf8_input();
std::vector<uint16_t> input_as_utf16(input.size()); // Reserve some space.
auto const result = std::uc::transcode_to_utf16(input, input_as_utf16.data());
input_as_utf16.resize(result.out - input_as_utf16.data()); // Trim unused space.
```

## Case 3: Transcode an object in as conveniently as possible

This solution is similar to Case 2, but marginally more convenient.  There are
other cases, like accepting output from `std` algorithms, that indicate use of
a back-inserter.

```cpp
struct my_string; // Some string type with *non-pointer* iterators.

my_string input = get_utf8_input();
std::vector<uint16_t> input_as_utf16;
std::ranges::copy(input, std::uc::from_utf8_back_inserter(input_as_utf16));
```

## Case 4: Adapt to an existing iterator interface taking a different UTF

This time, we have a generic iterator interface we want to transcode into, so
we want to use the transcoding iterators.

```cpp
// A generic function that accepts sequences of UTF-16.
template<typename UTF16Iter>
void process_input(UTF16Iter first, UTF16Iter last);

std::string input = get_utf8_input(); // A std::string used as a UTF-8 string.

process_input(std::uc::utf_8_to_16_iterator(input.begin(), input.begin(), input.end()),
              std::uc::utf_8_to_16_iterator(input.begin(), input.end(), input.end()));

#if 0 // or, we could have done:
auto const utf16_view = std::uc::as_utf16(input);
process_input(utf16_view.begin(), utf16.end());
#endif
```

## Case 5: Adapt to an existing range interface taking a different UTF

In this case, we have a generic range interface to transcode into, so we use a
transcoding view.

```cpp
// A generic function that accepts sequences of UTF-16.
template<typename UTF16Range>
void process_input(UTF16Range && r);

std::string input = get_utf8_input(); // A std::string used as a UTF-8 string.

process_input(std::uc::as_utf16(input));
```

# Proposed design

Add concepts that describe parameters to transcoding APIs.

```cpp
namespace std::uc {

  enum class format { utf8 = 1, utf16 = 2, utf32 = 4 };

  template<integral T>
    requires (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4)
      constexpr format @*format-of*@() // @*exposition only*@
      {
          constexpr format formats[] = {
              format::utf8,
              format::utf8,
              format::utf16,
              format::utf32,
              format::utf32};
          return formats[sizeof(T)];
      }

  template<typename T, format F>
    concept code_unit = integral<T> && sizeof(T) == (int)F;

  template<typename T>
    concept utf8_code_unit = code_unit<T, format::utf8>;

  template<typename T>
    concept utf16_code_unit = code_unit<T, format::utf16>;

  template<typename T>
    concept utf32_code_unit = code_unit<T, format::utf32>;

  template<typename T, format F>
    concept code_unit_iter =
      bidirectional_iterator<T> && code_unit<iter_value_t<T>, F>;

  template<typename T, format F>
    concept code_unit_pointer =
      is_pointer_v<T> && code_unit<iter_value_t<T>, F>;

  template<typename T>
    concept utf8_iter = code_unit_iter<T, format::utf8>;
  template<typename T>
    concept utf8_pointer = code_unit_pointer<T, format::utf8>;

  template<typename T>
    concept utf16_iter = code_unit_iter<T, format::utf16>;
  template<typename T>
    concept utf16_pointer = code_unit_pointer<T, format::utf16>;

  template<typename T>
    concept utf32_iter = code_unit_iter<T, format::utf32>;
  template<typename T>
    concept utf32_pointer = code_unit_pointer<T, format::utf32>;

  template<typename T>
    concept code_point = utf32_code_unit<T>;
  template<typename T>
    concept code_point_iter = utf32_iter<T>;

  template<typename T>
    concept utf_iter = utf8_iter<T> || utf16_iter<T> || utf32_iter<T>;

  template<typename T>
    concept utf_range_like =
      utf8_range<remove_reference_t<T>> ||
      utf16_range<remove_reference_t<T>> ||
      utf32_range<remove_reference_t<T>> ||
      utf8_pointer<remove_reference_t<T>> ||
      utf16_pointer<remove_reference_t<T>> ||
      utf32_pointer<remove_reference_t<T>>;
}
```

Add a standard null-terminated sequence sentinel:

```cpp
namespace std::uc {
  struct null_sentinel_t
  {
    constexpr null_sentinel_t base() const { return {}; }

    template<typename T>
      requires utf8_code_unit<T> || utf16_code_unit<T> || utf32_code_unit<T>
        friend constexpr auto operator==(T * p, null_sentinel_t);
  };

  inline constexpr null_sentinel_t null_sentinel;
}
```

Add constants and utility functions that query the state of UTF sequences
(well-formedness, etc.):

```cpp
namespace std::uc {
  inline constexpr uint16_t high_surrogate_base = 0xd7c0;
  inline constexpr uint16_t low_surrogate_base = 0xdc00;
  inline constexpr uint32_t high_surrogate_min = 0xd800;
  inline constexpr uint32_t high_surrogate_max = 0xdbff;
  inline constexpr uint32_t low_surrogate_min = 0xdc00;
  inline constexpr uint32_t low_surrogate_max = 0xdfff;
  inline constexpr uint32_t replacement_character = 0xfffd;

  // Returns is_high_surrogate(c) || is_low_surrogate(c).
  constexpr bool is_surrogate(uint32_t c);

  // Returns true iff c is a Unicode high surrogate.
  constexpr bool is_high_surrogate(uint32_t c);

  // Returns true iff c is a Unicode low surrogate.
  constexpr bool is_low_surrogate(uint32_t c);

  // Returns true iff c is a Unicode reserved noncharacter.
  constexpr bool is_reserved_noncharacter(uint32_t c);

  // Returns true iff c is a valid Unicode scalar value.
  constexpr bool is_scalar_value(uint32_t c);

  // Returns true iff c is a Unicode scalar value not in the reserved
  // range.
  constexpr bool is_unreserved_scalar_value(uint32_t c);

  // Returns true iff c is a UTF-8 lead code unit (which must be followed
  // by 1-3 following units).
  constexpr bool is_lead_code_unit(unsigned char c);

  // Returns true iff c is a UTF-8 continuation (non-lead) code unit.
  constexpr bool is_continuation(unsigned char c);

  // Given the first (and possibly only) code unit of a UTF-8-encoded code
  // point, returns the number of bytes occupied by that code point (in the
  // range [1, 4]).  Returns a value < 0 if first_unit is not a valid
  // initial UTF-8 code unit.
  constexpr int utf8_code_units(unsigned char first_unit);

  // Given the first (and possibly only) code unit of a UTF-16-encoded code
  // point, returns the number of code units occupied by that code point
  // (in the range [1, 2]).  Returns a value < 0 if first_unit is
  // not a valid initial UTF-16 code unit.
  constexpr int utf16_code_units(uint16_t first_unit);

  // Returns the first code unit in [first, last) that is not properly
  // UTF-8 encoded, or last if no such code unit is found.
  template<utf8_iter I, sentinel_for<I> S = I>
    requires random_access_iterator<I>
      constexpr I find_invalid_encoding(I first, S last);

  // Returns the first code unit in [first, last) that is not properly
  // UTF-16 encoded, or last if no such code unit is found.
  template<utf16_iter I, sentinel_for<I> S = I>
    requires random_access_iterator<I>
      constexpr I find_invalid_encoding(I first, S last);

  // Returns true iff [first, last) is properly UTF-8 encoded.
  template<utf8_iter I>
    requires random_access_iterator<I>
      constexpr bool is_encoded(I first, I last);

  // Returns true iff [first, last) is properly UTF-16 encoded
  template<utf16_iter I>
    requires random_access_iterator<I>
      constexpr bool is_encoded(I first, I last);

  // Returns true iff [first, last) is empty or the initial UTF-8 code
  // units in [first, last) form a valid Unicode code point.
  template<utf8_iter I>
    requires random_access_iterator<I>
      constexpr bool starts_encoded(I first, I last);

  // Returns true iff [first, last) is empty or the initial UTF-16 code
  // units in [first, last) form a valid Unicode code point.
  template<utf16_iter I>
    requires random_access_iterator<I>
      constexpr bool starts_encoded(I first, I last);

  // Returns true iff [first, last) is empty or the final UTF-8 code
  // units in [first, last) form a valid Unicode code point.
  template<utf8_iter I>
    requires random_access_iterator<I>
      constexpr bool ends_encoded(I first, I last);

  // Returns true iff [first, last) is empty or the final UTF-16 code
  // units in [first, last) form a valid Unicode code point.
  template<utf16_iter I>
    requires random_access_iterator<I>
      constexpr bool ends_encoded(I first, I last);
}
```

Add transcode algorithms:

```cpp
namespace std::uc {

  // An alias for in_out_result returned by algorithms that perform a
  // transcoding copy.
  template<typename Iter, typename OutIter>
  using transcode_result = in_out_result<Iter, OutIter>;

  // -> UTF-8

  template<
    input_iterator I,
    sentinel_for<I> S,
    output_iterator<uint8_t> O>
    requires (utf16_code_unit<iter_value_t<I>> ||
              utf32_code_unit<iter_value_t<I>>)
  transcode_result<I, O> transcode_to_utf8(I first, S last, O out);

  // equivalent to transcode_to_utf8(p, null_sentinel, out);
  template<typename Ptr, output_iterator<uint8_t> O>
    requires (utf16_pointer<Ptr> || utf32_pointer<Ptr>)
  transcode_result<Ptr, O> transcode_to_utf8(Ptr p, O out);

  template<ranges::input_range R, output_iterator<uint8_t> O>
    requires (utf16_code_unit<ranges::range_value_t<R>> ||
              utf32_code_unit<ranges::range_value_t<R>>)
  transcode_result<ranges::borrowed_iterator_t<R>, O>
  transcode_to_utf8(R && r, O out);

  // -> UTF-16

  template<
    input_iterator I,
    sentinel_for<I> S,
    output_iterator<uint16_t> O>
    requires (utf8_code_unit<iter_value_t<I>> ||
              utf32_code_unit<iter_value_t<I>>)
  transcode_result<I, O> transcode_to_utf16(I first, S last, O out);

  // equivalent to transcode_to_utf16(p, null_sentinel, out);
  template<typename Ptr, output_iterator<uint16_t> O>
    requires (utf8_pointer<Ptr> || utf32_pointer<Ptr>)
  transcode_result<Ptr, O> transcode_to_utf16(Ptr p, O out);

  template<ranges::input_range R, output_iterator<uint16_t> O>
    requires (utf8_code_unit<ranges::range_value_t<R>> ||
              utf32_code_unit<ranges::range_value_t<R>>)
  transcode_result<ranges::borrowed_iterator_t<R>, O>
  transcode_to_utf16(R && r, O out);

  // -> UTF-32

  template<
    input_iterator I,
    sentinel_for<I> S,
    output_iterator<uint32_t> O>
    requires (utf8_code_unit<iter_value_t<I>> ||
              utf16_code_unit<iter_value_t<I>>)
  transcode_result<I, O> transcode_to_utf32(I first, S last, O out);

  // equivalent to transcode_to_utf32(p, null_sentinel, out);
  template<typename Ptr, output_iterator<uint32_t> O>
    requires (utf8_pointer<Ptr> || utf16_pointer<Ptr>)
  transcode_result<Ptr, O> transcode_to_utf32(Ptr p, O out);

  template<ranges::input_range R, output_iterator<uint32_t> O>
    requires (utf8_code_unit<ranges::range_value_t<R>> ||
              utf16_code_unit<ranges::range_value_t<R>>)
  transcode_result<ranges::borrowed_iterator_t<R>, O>
  transcode_to_utf32(R && r, O out);

}
```

Optional: Add the array overloads of the transcode algorithms:

These are not strictly necessary, and only cover one corner case.  They cover
the transcoding of a non-null-terminated array.  I'm not sure they're worth
it.  Without these, the call `transcode_to_utfN(arr, out)` will choose the
`transcode_to_utfN()` pointer overload, due to array-to-pointer decay.  So,
having these overloads increases safety.

```cpp
  template<size_t N, typename Char, output_iterator<uint8_t> O>
    requires (utf16_code_unit<Char> || utf32_code_unit<Char>)
    transcode_result<Char *, O> transcode_to_utf8(Char (&arr)[N], O out);

  template<size_t N, typename Char, output_iterator<uint16_t> O>
    requires (utf8_code_unit<Char> || utf32_code_unit<Char>)
    transcode_result<Char *, O> transcode_to_utf16(Char (&arr)[N], O out);

  template<size_t N, typename Char, output_iterator<uint32_t> O>
    requires (utf8_code_unit<Char> || utf16_code_unit<Char>)
    transcode_result<Char *, O> transcode_to_utf32(Char (&arr)[N], O out);
```

Add the transcoding iterators (first, the basic ones).  I'm using
[P2727](https://isocpp.org/files/papers/P2727R0.html)'s `iterator_interface`
here for simplicity.

```cpp
    // An error handler type that can be used with the converting iterators;
    // provides the Unicode replacement character on errors.
    struct use_replacement_character
    {
        constexpr uint32_t operator()(char const *) const
        { return replacement_character; }
    };

    // UTF-32 -> UTF-8
    template<
        utf32_iter I,
        sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    struct utf_32_to_8_iterator
        : iterator_interface<utf_32_to_8_iterator<I, S, ErrorHandler>,
                             bidirectional_iterator_tag,
                             char,
                             char>
    {
        constexpr utf_32_to_8_iterator();
          explicit constexpr utf_32_to_8_iterator(I first, I it, S last);
        template<typename I2, typename S2>
          requires convertible_to<I2, I> && convertible_to<S2, S>
            constexpr utf_32_to_8_iterator(
              utf_32_to_8_iterator<I2, S2, ErrorHandler> const & other);

        constexpr I begin() const { return first_; }
        constexpr S end() const { return last_; }

        constexpr char operator*() const { return buf_[index_]; }

        constexpr I base() const { return it_; }

        constexpr utf_32_to_8_iterator & operator++();
        constexpr utf_32_to_8_iterator & operator--();

        template<
          typename I1,
          typename S1,
          typename I2,
          typename S2,
          typename ErrorHandler2>
        friend constexpr bool operator==(
          utf_32_to_8_iterator<I1, S1, ErrorHandler2> const & lhs,
          utf_32_to_8_iterator<I2, S2, ErrorHandler2> const & rhs)
            requires requires { lhs.base() == rhs.base(); };

        friend bool operator==(utf_32_to_8_iterator lhs, utf_32_to_8_iterator rhs)
          { return lhs.base() == rhs.base() && lhs.index_ == rhs.index_; }

        using base_type =
          detail::trans_iter<utf_32_to_8_iterator<I, S, ErrorHandler>, char>;
        using base_type::operator++;
        using base_type::operator--;

    private:
        I first_;                 // @*exposition only*@
        I it_;                    // @*exposition only*@
        S last_;                  // @*exposition only*@
        int index_;               // @*exposition only*@
        array<char, 5> buf_;      // @*exposition only*@

        template<
          utf32_iter I2,
          sentinel_for<I2> S2,
          transcoding_error_handler ErrorHandler2>
        friend struct utf_32_to_8_iterator;

#endif
    };

    template<typename I, typename S, typename ErrorHandler>
      constexpr bool operator==(
        utf_32_to_8_iterator<I, S, ErrorHandler> lhs, S rhs)
          requires requires { lhs.base() == rhs; };





```

TODO

## Design notes

None of the proposed interfaces is subject to change in future versions of
Unicode; each is from the guaranteed-stable subset.  Just sayin'.

None of the proposed interfaces allocates.

The proposed interfaces allow users to choose amongst multiple
convenience-vs-compatibility-vs-performance tradeoffs.  Explicitly, they are:

- If you need maximum performance, stick to the transcoding algorithms, and in
  particular use pointers for input and output.
- If you need compatibility with existing iterator-based algorithms (such as
  the standard algorithms), use the transcoding iterators.
- If you want streamability or the convenience of constructing ranges with a
  single `as_utfN()` function call, use the transcoding views.

All the transcoding iterators allow you access to the underlying iterator via
`.base()`, following the convention of the iterator adaptors already in the
standard.

The transcoding views are lazy, as you'd expect.  They also compose with the
standard view adaptors, so just transcoding at most 10 UTF-16 code units out
of some UTF can be done with `std::uc::as_utf16(foo) |
std::ranges::views::take(10)`.

Error handling is explicitly configurable in the transcoding iterators.  This
gives complete control to those who want to do something other than the
default.  The default, according to Unicode, is to produce a replacement
character (`0xfffd`) in the output when broken UTF encoding is seen in the
input.  This is what all these interfaces do, unless you configure one of the
iterators as mentioned above.

Code units are just numbers.  All of these interfaces treat integral types as
code units of various sizes (as least the ones that are 8-, 16-, or 32-bit).
Signedness is ignored.

A null-terminated pointer `p` to an 8-, 16-, or 32-bit string of code units is
considered the implicit range `[p, null_sentinel)`.  This makes user code much
more natural; `as_utf16("foo")`, `as_utf16("foo"sv)`, and `as_utf16("foo"s)`
are all equivalent.

Iterators are constructed from more than one underlying iterator.  To do
iteration in many text-handling contexts, you need to know the beginning and
the end of the range you are iterating over, just to be able to do iteration
correctly. Note that this is not a safety issue, but a correctness one.  For
example, say we have a string `s` of UTF-8 code units that we would like to
iterate over to produce UTF-32 code points. If the last code unit in `s` is
`0xe0`, we should expect two more code units to follow. They are not present,
though, becuase `0xe0` is the last code unit. Now consider how you would
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

### Iterator "unpacking"

A simple way to represent a transcoding view is as a pair of transcoding
iterators. However, there is a problem with that approach, since a
`utf32_view<utf_8_to_32_iterator<char const *>>` would be a range the size of
6 pointers. Worse yet, a
`utf32_view<utf_8_to_16_iterator<utf_16_to_32_iterator<char const *>>>` would
be the size of 18 pointers! Further, such a view would do a UTF-8 to UTF-16 to
UTF-32 conversion, when it could have done a direct UTF-8 to UTF-32 conversion
instead.

To solve these kinds of problems, `as_utfN()` unpacks the iterators it is given,
so that only the bottom-most underlying pointer or iterator is stored:

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

auto range = std::uc::as_utf8(to_32_first, to_32_last);

// Poof!  The utf_16_to_32_iterators disappeared!
static_assert(std::is_same<decltype(range),
                           std::uc::utf8_view<std::string::iterator>>::value, "");
```

Each of these views stores only the unpacked iterator and sentinel, so each
view is typically the size of two pointers, and possibly smaller if a sentinel
is used.

The same unpacking logic is used in `utfN_iterator()`, `from_utfN_inserter()`,
the transcoding algorithms, and the normalization algorithms. This allows you
to write `std::uc::as_utf32(first, last)` in a generic context, without caring
whether first and last are iterators to a sequence of UTF-8, UTF-16, or
UTF-32. You also do not need to care about whether first and last are raw
pointers, some other kind of iterator, or transcoding iterators. For example,
if first is a `utf_32_to_8_iterator`, the resulting view will use
`first.base()` for its begin-iterator.

# Performance

The performance situation for UTF transcoding is complicated, and so bears
some discussion. All the charts below were generated using Google Benchmark,
built with GCC on Linux.

## UTF-8 to UTF-16

Here are the relative timings for UTF-8 to UTF-16 transcoding, using various
methods (smaller is better). The input was around half a megabyte of text from
Wikipedia. "Iterators" is using `std::copy` from `utf_8_to_16_iterator` to a
pointer; "Algorithm `std::back_inserter`" is using `transcode_to_utf16()` in
the SIMD code path, and outputting to a `std::back_insert_iterator`;
"Algorithm using SIMD" is using `transcode_to_utf16()` from pointer to pointer
in the SIMD-using code path; "Algorithm no SIMD" is using
`transcode_to_utf16()` from pointer to pointer in the non-SIMD code path; and
"ICU" is is using `UnicodeString::fromUTF8()`.

```<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg version="1.1" xmlns="http://www.w3.org/2000/svg" width="500" height="150">
    <rect width="160.13028" height="20" y="0" fill="#32a852"></rect>
    <text x="165.13028" y="10.0" dy=".35em" fill="black">Iterators</text>
    <rect width="90.00108" height="20" y="25" fill="#3f6078"></rect>
    <text x="95.00108" y="35.0" dy=".35em" fill="black">Algorithm std::back_inserter</text>
    <rect width="56.72772" height="20" y="50" fill="#ad3939"></rect>
    <text x="61.72772" y="60.0" dy=".35em" fill="black">Algorithm using SIMD</text>
    <rect width="70.77636" height="20" y="75" fill="#ad9e39"></rect>
    <text x="75.77636" y="85.0" dy=".35em" fill="black">Algorithm no SIMD</text>
    <rect width="57.12672" height="20" y="100" fill="#ba45cc"></rect>
    <text x="62.12672" y="110.0" dy=".35em" fill="black">ICU</text>
    <rect width="300" height="20" y="125" fill="white"></rect>
    <line x1="1" y1="129" x2="300" y2="129" stroke="black" fill="black"></line>
    <line x1="1" y1="124" x2="1" y2="134" stroke="black" fill="black"></line>
    <line x1="300" y1="124" x2="300" y2="134" stroke="black" fill="black"></line>
    <text x="0" y="144.0" dy=".35em" fill="black">0 ns</text>
    <text x="300" y="144.0" dy=".35em" text-anchor="end" fill="black">2500000 ns</text>
</svg>```{=html}

The ICU performance is shown as something of a baseline, given the ubiquity of
ICU's use in Unicode-aware programs. Note that ICU does not have convenient
APIs for doing transcoding to any format but UTF-16.

There are some take-always from this chart (and in fact all the other
transcoding data):

- The use of SIMD instructions is helpful, but not critical.
- The use of back-inserters is quite bad for performance.
- The transcoding iterators are terrible for performance.
- All the above only apply to transcode-only operations; more complicated
  operations that involve a transcoding step are often fairly insensitive to
  transcoding performance.
- The fastest API proposed is as fast as the equivalent ICU API.

A major reason for the performance differences is that the fastest algorithms
are able to write out chunks of their results all in one go (up to 16 at once
in the SIMD paths of the transcode algorithms). Needing to branch on each
output code unit as in the "Iterators" and "Algorithm `std::back_inserter`"
cases is much slower. One implication of this is that if you're doing a lot of
work with each code unit or code point produced, you're probably doing a lot
of branching in the work, and so the gains of using the high-performance
methods above will be lost. Specifically, passing transcoding iterators to
complicated Unicode algorithms like the Bidirectional Algorithm do not result
in much (if any) performance loss.

## UTF-8 to UTF-32

These are relative timings for UTF-8 to UTF-32 transcoding. It is in the same
scale as the chart above.

```<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg version="1.1" xmlns="http://www.w3.org/2000/svg" width="500" height="125">
    <rect width="99.09804" height="20" y="0" fill="#32a852"></rect>
    <text x="104.09804" y="10.0" dy=".35em" fill="black">Iterators</text>
    <rect width="100.27296" height="20" y="25" fill="#3f6078"></rect>
    <text x="105.27296" y="35.0" dy=".35em" fill="black">Algorithm std::back_inserter</text>
    <rect width="40.50972" height="20" y="50" fill="#ad3939"></rect>
    <text x="45.50972" y="60.0" dy=".35em" fill="black">Algorithm using SIMD</text>
    <rect width="54.67824" height="20" y="75" fill="#ad9e39"></rect>
    <text x="59.67824" y="85.0" dy=".35em" fill="black">Algorithm no SIMD</text>
    <rect width="300" height="20" y="100" fill="white"></rect>
    <line x1="1" y1="104" x2="300" y2="104" stroke="black" fill="black"></line>
    <line x1="1" y1="99" x2="1" y2="109" stroke="black" fill="black"></line>
    <line x1="300" y1="99" x2="300" y2="109" stroke="black" fill="black"></line>
    <text x="0" y="119.0" dy=".35em" fill="black">0 ns</text>
    <text x="300" y="119.0" dy=".35em" text-anchor="end" fill="black">2500000 ns</text>
</svg>```{=html}

Again, you can see very similar relationships among the different transcoding
methods, except that the iterator method is a lot faster.

Note that the SIMD algorithm is quite fast. It — and all the SIMD code — was
originally developed by Bob Steagall, and presented at C++Now in 2018. Thanks,
Bob!
