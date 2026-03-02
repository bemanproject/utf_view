# beman.utf_view: C++29 UTF Transcoding Views

<!--
SPDX-License-Identifier: BSL-1.0

  Copyright Eddie Nolan and Jonathan Wakely 2023 - 2025.
Distributed under the Boost Software License, Version 1.0.
   (See accompanying file LICENSE.txt or copy at
         https://www.boost.org/LICENSE_1_0.txt)
-->

![Library Status](https://raw.githubusercontent.com/bemanproject/beman/refs/heads/main/images/badges/beman_badge-beman_library_under_development.svg) ![Continuous Integration Tests](https://github.com/bemanproject/utf_view/actions/workflows/ci_tests.yml/badge.svg) ![Lint Check (pre-commit)](https://github.com/bemanproject/utf_view/actions/workflows/pre-commit.yml/badge.svg) [![Coverage](https://coveralls.io/repos/github/bemanproject/utf_view/badge.svg?branch=main)](https://coveralls.io/github/bemanproject/utf_view?branch=main) ![Standard Target](https://github.com/bemanproject/beman/blob/main/images/badges/cpp29.svg)

 C++29 UTF transcoding features:
- Transcoding UTF views `to_utf8`, `to_utf16`, and `to_utf32`
- `null_sentinel` sentinel and `null_term` CPO for creating views of null-terminated strings
- Casting views for creating views of `charN_t`, which are `as_char8`, `as_char16`, `as_char32`

**Implements**: [Unicode in the Library, Part 1: UTF Transcoding (P2728R10)](https://isocpp.org/files/papers/P2728R10.html), [A Sentinel for Null-Terminated Strings (P3705R2)](https://isocpp.org/files/papers/P3705R2.html), and [Endian Views (P4030R0)](https://isocpp.org/files/papers/P4030R0.html)

**Status**: [Under development and not yet ready for production use.](https://github.com/bemanproject/beman/blob/main/docs/beman_library_maturity_model.md#under-development-and-not-yet-ready-for-production-use)

## License

beman.utf_view is licensed under BSL 1.0.

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
- Clang 21/libc++
- MSVC 19.41

Note: Building with Clang and libstdc++ is not currently supported due to various bugs.

## Development

### Develop using GitHub Codespace

This project supports [GitHub Codespace](https://github.com/features/codespaces)
via [Development Containers](https://containers.dev/),
which allows rapid development and instant hacking in your browser.
We recommend using GitHub codespace to explore this project as it
requires minimal setup.

Click the following badge to create a codespace:

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://codespaces.new/bemanproject/utf_view)

For more documentation on GitHub codespaces, please see
[this doc](https://docs.github.com/en/codespaces/).

> [!NOTE]
>
> The codespace container may take up to 5 minutes to build and spin-up; this is normal.

### Develop locally on your machines

<details>
<summary> For Linux </summary>

Beman libraries require [recent versions of CMake](#build-environment),
we recommend downloading CMake directly from [CMake's website](https://cmake.org/download/)
or installing it with the [Kitware apt library](https://apt.kitware.com/).

A [supported compiler](#supported-platforms) should be available from your package manager.

</details>

<details>
<summary> For MacOS </summary>

Beman libraries require [recent versions of CMake](#build-environment).
Use [`Homebrew`](https://brew.sh/) to install the latest version of CMake.

```bash
brew install cmake
```

A [supported compiler](#supported-platforms) is also available from brew.

For example, you can install the latest major release of Clang as:

```bash
brew install llvm
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

Note that you will need to use FetchContent to build GoogleTest. To do so,
please see the instructions in the "Build GoogleTest dependency from github.com"
dropdown in the [Project specific configure
arguments](#project-specific-configure-arguments) section.

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
instrumentation enabled and many sanitizers enabled.

> [!NOTE]
>
> The sanitizers that are enabled vary from compiler to compiler.
> See the toolchain files under ([`cmake`](cmake/)) to determine the exact configuration used for each preset.

The `release` presets are designed for production use, and
consequently have the highest optimization turned on (e.g. `O3`).

### Configure and Build Manually

If the presets are not suitable for your use-case, a traditional CMake
invocation will provide more configurability.

To configure, build and test the project with extra arguments,
you can run this set of commands.

```bash
cmake \
  -B build \
  -S . \
  -DCMAKE_CXX_STANDARD=20 \
  -DCMAKE_PREFIX_PATH=$PWD/infra/cmake \
  # Your extra arguments here.
cmake --build build
ctest --test-dir build
```

> [!IMPORTANT]
>
> Beman projects are
> [passive projects](https://github.com/bemanproject/beman/blob/main/docs/beman_standard.md#cmake),
> therefore,
> you will need to specify the C++ version via `CMAKE_CXX_STANDARD`
> when manually configuring the project.

### Project specific configure arguments

Project-specific options are prefixed with `BEMAN_UTF_VIEW`.
You can see the list of available options with:

```bash
cmake -LH -S . -B build | grep "BEMAN_UTF_VIEW" -C 2
```

<details>

<summary> Details of CMake arguments. </summary>

#### `BEMAN_UTF_VIEW_BUILD_TESTS`

Enable building tests and test infrastructure. Default: ON.
Values: `{ ON, OFF }`.

You can configure the project to have this option turned off via:

```bash
cmake -B build -S . -DCMAKE_CXX_STANDARD=20 -DBEMAN_UTF_VIEW_BUILD_TESTS=OFF
```

#### `BEMAN_UTF_VIEW_BUILD_EXAMPLES`

Enable building examples. Default: ON. Values: { ON, OFF }.

#### `BEMAN_UTF_VIEW_INSTALL_CONFIG_FILE_PACKAGE`

Enable installing the CMake config file package. Default: ON.
Values: { ON, OFF }.

This is required so that users of `beman.utf_view` can use
`find_package(beman.utf_view)` to locate the library.

#### `BEMAN_UTF_VIEW_BUILD_PAPER`

Enable building the HTML version of P2728 and P3705 from their markdown sources. Default: ON. Values: { ON, OFF }.

</details>

## Integrate beman.utf_view into your project

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

The process for incorporating `beman.utf_view` into your project depends on the
build system being used. Instructions for CMake are provided in following sections.

### Incorporating `beman.utf_view` into your project with CMake

For CMake based projects,
you will need to use the `beman.utf_view` CMake module
to define the `beman::utf_view` CMake target:

```cmake
find_package(beman.utf_view REQUIRED)
```

You will also need to add `beman::utf_view` to the link libraries of
any libraries or executables that include `beman.utf_view` headers.

```cmake
target_link_libraries(yourlib PUBLIC beman::utf_view)
```

### Produce beman.utf_view interface library

You can produce utf_view's interface library locally by:

```bash
cmake --workflow --preset gcc-release
cmake --install build/gcc-release --prefix /opt/beman
```

This will generate the following directory structure at `/opt/beman`.

```txt
/opt/beman
├── include
│   └── beman
│       └── utf_view
│           ├── // ...
│           └── utf_view.hpp
└── lib
    └── cmake
        └── beman.utf_view
            ├── beman.utf_view-config-version.cmake
            ├── beman.utf_view-config.cmake
            └── beman.utf_view-targets.cmake
```

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
