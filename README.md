# beman.utf_view: C++29 UTF Transcoding Views

<!--
SPDX-License-Identifier: BSL-1.0

  Copyright Eddie Nolan 2023 - 2026.
Distributed under the Boost Software License, Version 1.0.
   (See accompanying file LICENSE.txt or copy at
         https://www.boost.org/LICENSE_1_0.txt)
-->

<!-- markdownlint-disable-next-line line-length -->
![Library Status](https://raw.githubusercontent.com/bemanproject/beman/refs/heads/main/images/badges/beman_badge-beman_library_under_development.svg) ![Continuous Integration Tests](https://github.com/bemanproject/utf_view/actions/workflows/ci_tests.yml/badge.svg) ![Lint Check (pre-commit)](https://github.com/bemanproject/utf_view/actions/workflows/pre-commit-check.yml/badge.svg) [![Coverage](https://coveralls.io/repos/github/bemanproject/utf_view/badge.svg?branch=main)](https://coveralls.io/github/bemanproject/utf_view?branch=main) ![Standard Target](https://github.com/bemanproject/beman/blob/main/images/badges/cpp29.svg)

 C++29 UTF transcoding features:

- Transcoding UTF views `to_utf8`, `to_utf16`, and `to_utf32`
- `null_sentinel` sentinel and `null_term` CPO for creating views of null-terminated strings
- Casting views for creating views of `charN_t`, which are `as_char8`, `as_char16`, `as_char32`

**Implements**: [Unicode in the Library, Part 1: UTF Transcoding (P2728R10)](https://isocpp.org/files/papers/P2728R10.html), [A Sentinel for Null-Terminated Strings (P3705R2)](https://isocpp.org/files/papers/P3705R2.html), and [Endian Views (P4030R0)](https://isocpp.org/files/papers/P4030R0.html)

**Status**: [Under development and not yet ready for production use.](https://github.com/bemanproject/beman/blob/main/docs/beman_library_maturity_model.md#under-development-and-not-yet-ready-for-production-use)

## License

beman.utf_view is licensed under the Boost Software License 1.0.

## Examples

Transcoding a UTF-8 string literal to a `std::u32string`:

```cpp
std::u32string hello_world =
  u8"こんにちは世界"sv | beman::utf_view::to_utf32 | std::ranges::to<std::u32string>();
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

Full runnable examples can be found in [`examples/`](examples/).

## Dependencies

### Software

beman.utf_view depends on [beman.transform_view_26](https://github.com/tzlaine/transform_view_26/). It brings in this library via CMake FetchContent.

### Build Environment

This project requires at least the following to build:

* A C++ compiler that conforms to the C++23 standard or greater
* CMake 3.30 or later

You can disable building tests by setting CMake option `BEMAN_UTF_VIEW_BUILD_TESTS` to
`OFF` when configuring the project.

### Supported Platforms

| Compiler | Version | C++ Standards | Standard Library |
|----------|---------|---------------|------------------|
| GCC      | 16-14   | C++26, C++23  | libstdc++        |
| GCC      | trunk   | C++26, C++23  | libstdc++        |
| Clang    | 22-19   | C++26, C++23  | libc++           |
| Clang    | trunk   | C++26, C++23  | libc++           |
| MSVC     | latest  | C++23         | MSVC STL         |

## Development

See the [Contributing Guidelines](CONTRIBUTING.md).

## Integrate beman.utf_view into your project

### Build

You can build utf_view using a CMake workflow preset:

```bash
cmake --workflow --preset gcc-release
```

To list available workflow presets, you can invoke:

```bash
cmake --list-presets=workflow
```

For details on building beman.utf_view without using a CMake preset, refer to the
[Contributing Guidelines](CONTRIBUTING.md).

### Installation

#### Vcpkg

The preferred way to install utf_view is via vcpkg. To do so, after installing vcpkg
itself, you need to add support for the Beman project's [vcpkg
registry](https://github.com/bemanproject/vcpkg-registry) by configuring a
`vcpkg-configuration.json` file (which utf_view [provides](vcpkg-configuration.json)).

Then, simply run `vcpkg install beman-utf-view`.

#### Manual

To install beman.utf_view globally after building with the `gcc-release` preset, you can
run:

```bash
sudo cmake --install build/gcc-release
```

Alternatively, to install to a prefix, for example `/opt/beman`, you can run:

```bash
sudo cmake --install build/gcc-release --prefix /opt/beman
```

This will generate the following directory structure:

```txt
/opt/beman
├── include
│   └── beman
│       └── utf_view
│           ├── utf_view.hpp
│           └── ...
└── lib
    └── cmake
        └── beman.utf_view
            ├── beman.utf_view-config-version.cmake
            ├── beman.utf_view-config.cmake
            └── beman.utf_view-targets.cmake
```

### CMake Configuration

If you installed beman.utf_view to a prefix, you can specify that prefix to your CMake
project using `CMAKE_PREFIX_PATH`; for example, `-DCMAKE_PREFIX_PATH=/opt/beman`.

You need to bring in the `beman.utf_view` package to define the `beman::utf_view` CMake
target:

```cmake
find_package(beman.utf_view REQUIRED)
```

You will then need to add `beman::utf_view` to the link libraries of any libraries or
executables that include `beman.utf_view` headers.

```cmake
target_link_libraries(yourlib PUBLIC beman::utf_view)
```

### Using beman.utf_view

To use `beman.utf_view` in your C++ project,
include an appropriate `beman.utf_view` header from your source code.

```c++
#include <beman/utf_view/utf_view.hpp>
```

> [!NOTE]
>
> `beman.utf_view` headers are to be included with the `beman/utf_view/` prefix.
> Altering include search paths to spell the include target another way (e.g.
> `#include <utf_view.hpp>`) is unsupported.

## Paper

beman.utf_view is based on P2728 and P3705.

- The latest official revision of P2728 can be found at https://wg21.link/p2728
- The latest official revision of P3705 can be found at https://wg21.link/p3705
- The unofficial latest draft Markdown source for each paper can be found in this repository:
  - At [papers/P2828.md](https://github.com/bemanproject/utf_view/blob/main/papers/P2728.md)
  - At [papers/P3705.md](https://github.com/bemanproject/utf_view/blob/main/papers/P3705.md)
- P2728's committee status page can be found at https://github.com/cplusplus/papers/issues/1422

## Authors

The implementation of P2728 is a fork by Eddie Nolan of the implementation of P2728R6 in libstdc++ by Jonathan Wakely at [`gcc/libstdc++-v3/include/bits/unicode.h`](https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=libstdc%2B%2B-v3/include/bits/unicode.h;h=66f8399fdfb05d85fcdb37fa9ec7c4089feb7a7d;hb=37a4c5c23a27).
