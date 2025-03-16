<!--
SPDX-License-Identifier: BSL-1.0

  Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
Distributed under the Boost Software License, Version 1.0.
   (See accompanying file LICENSE.txt or copy at
         https://www.boost.org/LICENSE_1_0.txt)
-->

# beman.utf_view: C++26 UTF Transcoding Views

<img src="https://github.com/bemanproject/beman/blob/main/images/logos/beman_logo-beman_library_under_development.png" style="width:5%; height:auto;">[![CI](https://github.com/bemanproject/utf_view/actions/workflows/ci.yml/badge.svg)](https://github.com/bemanproject/utf_view/actions) [![Coverage](https://coveralls.io/repos/github/bemanproject/utf_view/badge.svg?branch=main)](https://coveralls.io/github/bemanproject/utf_view?branch=main)

 C++26 UTF transcoding features:
- Transcoding UTF views `to_utf8`, `to_utf16`, and `to_utf32`
- `null_sentinel` sentinel and `null_term` CPO for creating views of null-terminated strings
- Casting views for creating views of `charN_t`, which are `as_char8`, `as_char16`, `as_char32`

**Implements**: [Unicode in the Library, Part 1: UTF Transcoding
(P2728R7)](https://wg21.link/P2728R&)

**Status**: [Under development and not yet ready for production use.](https://github.com/bemanproject/beman/blob/main/docs/BEMAN_LIBRARY_MATURITY_MODEL.md#under-development-and-not-yet-ready-for-production-use)
  
## Examples

Transcoding a UTF-8 string literal to a `std::u32string`:

```cpp
std::u32string hello_world =
  u8"こんにちは世界" | beman::utf_view::to_utf32 | std::ranges::to<std::u32string>();
```

Sanitizing potentially invalid Unicode C strings by replacing invalid code units with replacement characters:

```cpp
template <typename CharT>
std::basic_string<CharT> sanitize(CharT const* str) {
  return beman::utf_view::null_term(str) | beman::utf_view::to_utf<CharT> | std::ranges::to<std::basic_string<CharT>>();
}
```

Returning the final non-ASCII code point in a string, transcoding backwards lazily:

```cpp
std::optional<char32_t> last_nonascii(std::ranges::view auto str) {
  for (auto c : str | beman::utf_view::to_utf32 | std::views::reverse 
                    | std::views::filter([](char32_t c) { return c > 0x7f; })
                    | std::views::take(1)) {
    return c;
  }
  return std::nullopt;
}
```

Transcoding strings and throwing a descriptive exception on invalid UTF:

(This example assumes the existence of the `enum_to_string` sample function
from [P2996](https://isocpp.org/files/papers/P2996R6.html#enum-to-string))

```cpp
template <typename FromChar, typename ToChar>
std::basic_string<ToChar> transcode_or_throw(std::basic_string_view<FromChar> input) {
  std::basic_string<ToChar> result;
  auto view = input | to_utf<ToChar>;
  for (auto it = view.begin(), end = view.end(); it != end; ++it) {
    if (it.success()) {
      result.push_back(*it);
    } else {
      throw std::runtime_error("error at position " +
                               std::to_string(it.base() - input.begin()) + ": " +
                               enum_to_string(it.success().error()));
    }
  }
  return result;
}
```

Changing the suits of Unicode playing card characters:

```cpp
enum class suit : std::uint8_t {
  spades = 0xA,
  hearts = 0xB,
  diamonds = 0xC,
  clubs = 0xD
};

// Unicode playing card characters are laid out such that changing the second least
// significant nibble changes the suit, e.g.
// U+1F0A1 PLAYING CARD ACE OF SPADES
// U+1F0B1 PLAYING CARD ACE OF HEARTS
constexpr char32_t change_playing_card_suit(char32_t card, suit s) {
  if (U'\N{PLAYING CARD ACE OF SPADES}' <= card && card <= U'\N{PLAYING CARD KING OF CLUBS}') {
    return (card & ~(0xF << 4)) | (static_cast<std::uint8_t>(s) << 4);
  }
  return card;
}

void change_playing_card_suits() {
  std::u8string_view const spades = u8"🂡🂢🂣🂤🂥🂦🂧🂨🂩🂪🂫🂭🂮";
  std::u8string const hearts =
    spades |
    to_utf32 |
    std::views::transform(std::bind_back(change_playing_card_suit, suit::hearts)) |
    to_utf8 |
    std::ranges::to<std::u8string>();
  assert(hearts == u8"🂱🂲🂳🂴🂵🂶🂷🂸🂹🂺🂻🂽🂾");
}
```

## How to Build

### Minimum Requirements

beman.utf_view requires being built in C++23 mode.

beman.utf_view has the following minimum compiler requirements:

- GCC 14
- Clang 19
- MSVC 19.41

### Dependencies

beman.utf_view depends on [beman.transform_view_26](https://github.com/tzlaine/transform_view_26/). It brings in this library via CMake FetchContent.

### Instructions

This excerpt from the project's CI script provides an example of building the project and running the tests:

    mkdir "$checkout/build"
    cd "$checkout/build"
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=23 -DBEMAN_UTF_VIEW_BUILD_TESTING=On "$@"
    cmake --build . --parallel
    ctest -C Debug

### Paper

beman.utf_view is based on P2728.

- The latest official revision of P2728 can be found at https://wg21.link/p2728
- The unofficial latest draft Markdown source for the paper can be found in this repository at [paper/P2828.md](https://github.com/bemanproject/utf_view/blob/main/paper/P2728.md)
- The paper's committee status page can be found at https://github.com/cplusplus/papers/issues/1422

## Authors

This implementation of P2728 is a fork by Eddie Nolan of the implementation of P2728R6 in libstdc++ by Jonathan Wakely at [`gcc/libstdc++-v3/include/bits/unicode.h`](https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=libstdc%2B%2B-v3/include/bits/unicode.h;h=66f8399fdfb05d85fcdb37fa9ec7c4089feb7a7d;hb=37a4c5c23a27).

## License

beman.utf_view is licensed under BSL 1.0.
