# UtfView: C++26 UTF Transcoding Views

<!--
SPDX-License-Identifier: GPL-3.0-only
-->

[![CI](https://github.com/ednolan/UtfView/actions/workflows/ci.yml/badge.svg)](https://github.com/ednolan/utfview/actions) [![Coverage](https://coveralls.io/repos/github/ednolan/UtfView/badge.svg?branch=main)](https://coveralls.io/github/ednolan/UtfView?branch=main)

This repository implements the functionality proposed by P2728 for C++26, including:

- Transcoding UTF views `to_utf8`, `to_utf16`, and `to_utf32`
- `null_sentinel` sentinel and `null_term` CPO for creating views of null-terminated strings
- Casting views for creating views of `charN_t`, which are `as_char8`, `as_char16`, `as_char32`
  
## Examples

Transcoding a UTF-8 string literal to a `std::u32string`:

```
std::u32string hello_world =
  u8"こんにちは世界" | utfview::to_utf32 | std::ranges::to<std::u32string>();
```

Sanitizing potentially invalid Unicode C strings by replacing invalid code units with replacement characters:

```
template <typename CharT>
std::basic_string<CharT> sanitize(CharT const* str) {
  return utfview::null_term(str) | utfview::to_utf<CharT> | std::ranges::to<std::basic_string<CharT>>();
}
```

Returning the final non-ASCII code point in a string, transcoding backwards lazily:

```
std::optional<char32_t> last_nonascii(std::ranges::view auto str) {
  for (auto c : str | utfview::to_utf32 | std::views::reverse 
                    | std::views::filter([](char32_t c) { return c > 0x7f; })
                    | std::views::take(1)) {
    return c;
  }
  return std::nullopt;
}
```

Transcoding strings and throwing a descriptive exception on invalid UTF:

(This example assumes the existence of the `enum_to_string` function from
[P2996](https://isocpp.org/files/papers/P2996R6.html#enum-to-string))

```
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

## How to Build

### Minimum Requirements

UtfView requires being built in C++23 mode.

UtfView has the following minimum compiler requirements:

- GCC 14
- Clang 19
- MSVC 17.11.2 (earlier MSVC versions may also work but are not tested in CI)

### Dependencies

UtfView depends on [Boost.STLInterfaces](https://github.com/boostorg/stl_interfaces). It brings in this library and its dependencies via git submodules. If you add UtfView to your project your project already uses Boost, it will target your project's stl_interfaces instead of using its own submodule.

### Instructions

This excerpt from the project's CI script provides an example of building the project and running the tests:

    mkdir "$checkout/build"
    cd "$checkout/build"
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=23 -DBUILD_TESTING=On "$@"
    cmake --build . --parallel
    ctest -C Debug

### Paper

UtfView is based on P2728.

- The latest official revision of P2728 can be found at https://wg21.link/p2728
- The unofficial latest draft Markdown source for the paper can be found in this repository at [paper/P2828.md](https://github.com/ednolan/UtfView/blob/main/paper/P2728.md)
- The paper's committee status page can be found at https://github.com/cplusplus/papers/issues/1422

## Authors

This implementation of P2728 is a fork by Eddie Nolan of the implementation of P2728R6 in libstdc++ by Jonathan Wakely at [`gcc/libstdc++-v3/include/bits/unicode.h`](https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=libstdc%2B%2B-v3/include/bits/unicode.h;h=66f8399fdfb05d85fcdb37fa9ec7c4089feb7a7d;hb=37a4c5c23a27).

## License

UtfView is licensed under GPLv3.
