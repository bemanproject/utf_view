<!--
SPDX-License-Identifier: BSL-1.0

  Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
Distributed under the Boost Software License, Version 1.0.
   (See accompanying file LICENSE.txt or copy at
         https://www.boost.org/LICENSE_1_0.txt)
-->

# beman.utf_view: C++26 UTF Transcoding Views

![Library Status](https://raw.githubusercontent.com/bemanproject/beman/refs/heads/main/images/badges/beman_badge-beman_library_under_development.svg) [![CI](https://github.com/bemanproject/utf_view/actions/workflows/ci.yml/badge.svg)](https://github.com/bemanproject/utf_view/actions) [![Coverage](https://coveralls.io/repos/github/bemanproject/utf_view/badge.svg?branch=main)](https://coveralls.io/github/bemanproject/utf_view?branch=main)

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





## Dependencies

### Software

beman.utf_view depends on [beman.transform_view_26](https://github.com/tzlaine/transform_view_26/). It brings in this library via CMake FetchContent.

### Build Environment

This project requires at least the following to build:

* C++23
* CMake 3.27

You can disable building tests by setting cmake option
[`BEMAN_UTF_VIEW_BUILD_TESTS`](#beman_exemplar_build_tests) to `OFF`
when configuring the project.

### Supported Platforms

This project officially supports:

- GCC 14
- GCC 15
- Clang 19/libc++
- Clang 20/libc++
- MSVC 19.41

Note: Building with Clang and libstdc++ is not currently supported due to various bugs.

## Development

### Develop locally on your machines

<details>
<summary> For Linux based systems </summary>

Beman libraries require [recent versions of CMake](#build-environment),
we advise you to download CMake directly from [CMake's website](https://cmake.org/download/)
or install it via the [Kitware apt library](https://apt.kitware.com/).

A [supported compiler](#supported-platforms) should be available from your package manager.
Alternatively you could use an install script from official compiler vendors.

Here is an example of how to install the latest stable version of clang
as per [the official LLVM install guide](https://apt.llvm.org/).

```bash
bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
```

</details>

<details>
<summary> For Windows </summary>

To build Beman libraries, you will need the MSVC compiler. MSVC can be obtained
by installing Visual Studio; the free Visual Studio 2022 Community Edition can
be downloaded from
[Microsoft](https://visualstudio.microsoft.com/vs/community/).

After Visual Studio has been installed, you can launch "Developer PowerShell for
VS 2022" by typing it into Windows search bar. This shell environment will
provide CMake, Ninja, and MSVC, allowing you to build the library and run the
tests.

</details>

### Configure and Build the Project Using CMake Presets

This project recommends using [CMake Presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
to configure, build and test the project.
Appropriate presets for major compilers have been included by default.
You can use `cmake --list-presets` to see all available presets.

Here is an example to invoke the `gcc-debug` preset.

```shell
cmake --workflow --preset gcc-debug
```

Generally, there are two kinds of presets, `debug` and `release`.

The `debug` presets are designed to aid development, so it has debugging
instrumentation enabled and as many sanitizers turned on as possible.

> [!NOTE]
>
> The set of sanitizer supports are different across compilers.
> You can checkout the exact set of compiler arguments by looking at the toolchain
> files under the [`cmake`](cmake/) directory.

The `release` presets are designed for use in production environments,
thus they have the highest optimization turned on (e.g. `O3`).

### Configure and Build Manually

While [CMake Presets](#configure-and-build-the-project-using-cmake-presets) are
convenient, you might want to set different configuration or compiler arguments
than any provided preset supports.

To configure, build and test the project with extra arguments,
you can run this set of commands.

```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 # Your extra arguments here.
cmake --build build
ctest --test-dir build
```

> [!IMPORTANT]
>
> Beman projects are
> [passive projects](https://github.com/bemanproject/beman/blob/main/docs/BEMAN_STANDARD.md#cmake),
> therefore,
> you will need to specify the C++ version via `CMAKE_CXX_STANDARD`
> when manually configuring the project.

### Project specific configure arguments

When configuring the project manually,
you can pass an array of project specific CMake configs to customize your build.

Project specific options are prefixed with `BEMAN_UTF_VIEW`.
You can see the list of available options with:

```bash
cmake -LH | grep "BEMAN_UTF_VIEW" -C 2
```

<details>

<summary> Details of CMake arguments. </summary>

#### `BEMAN_UTF_VIEW_BUILD_TESTS`

Enable building tests and test infrastructure. Default: ON.
Values: { ON, OFF }.

You can configure the project to have this option turned off via:

```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=23 -DBEMAN_UTF_VIEW_BUILD_TESTS=OFF
```

#### `BEMAN_UTF_VIEW_BUILD_PAPER`

Enable building the HTML version of P2728 from its markdown sources. Default: ON. Values: { ON, OFF }.

</details>

## Paper

beman.utf_view is based on P2728.

- The latest official revision of P2728 can be found at https://wg21.link/p2728
- The unofficial latest draft Markdown source for the paper can be found in this repository at [paper/P2828.md](https://github.com/bemanproject/utf_view/blob/main/paper/P2728.md)
- The paper's committee status page can be found at https://github.com/cplusplus/papers/issues/1422

## Authors

This implementation of P2728 is a fork by Eddie Nolan of the implementation of P2728R6 in libstdc++ by Jonathan Wakely at [`gcc/libstdc++-v3/include/bits/unicode.h`](https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=libstdc%2B%2B-v3/include/bits/unicode.h;h=66f8399fdfb05d85fcdb37fa9ec7c4089feb7a7d;hb=37a4c5c23a27).

## License

beman.utf_view is licensed under BSL 1.0.
