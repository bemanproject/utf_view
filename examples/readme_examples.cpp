// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan 2023 - 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <beman/utf_view/config.hpp>
#include <beman/utf_view/code_unit_view.hpp>
#include <beman/utf_view/detail/fake_inplace_vector.hpp>
#include <beman/utf_view/endian_view.hpp>
#include <beman/utf_view/null_term.hpp>
#include <beman/utf_view/to_utf_view.hpp>
#if BEMAN_UTF_VIEW_USE_MODULES()
import std;
#else
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iterator>
#include <optional>
#include <print>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <version>
#endif

namespace beman::utf_view::examples {

using namespace std::string_view_literals;

template <typename CharT>
std::basic_string<CharT> sanitize(CharT const* str) {
  return null_term(str) | to_utf<CharT> | std::ranges::to<std::basic_string<CharT>>();
}

std::optional<char32_t> last_nonascii(std::ranges::view auto str) {
  for (auto c : str | as_char8_t | to_utf32 | std::views::reverse |
           std::views::filter([](char32_t c) { return c > 0x7f; })) {
    return c;
  }
  return std::nullopt;
}

#ifdef _MSC_VER
bool windows_path() {
  std::vector<int> path_as_ints = {U'C', U':', U'\x00010000'};
  std::filesystem::path path =
      path_as_ints | as_char32_t | std::ranges::to<std::u32string>();
  const auto& native_path = path.native();
  if (native_path != std::wstring{L'C', L':', L'\xD800', L'\xDC00'}) {
    return false;
  }
  return true;
}
#endif

std::u8string as_char32_t_example() {
  auto get_icu_code_points = [] { return std::vector<int>{0x1F574, 0xFFFD}; };
  std::vector<int> input = get_icu_code_points();
  // This is ill-formed without the as_char32_t adaptation.
  auto input_utf8 = input | as_char32_t | to_utf8 | std::ranges::to<std::u8string>();
  return input_utf8;
}

std::string enum_to_string(utf_transcoding_error ec) {
  switch (ec) {
  case utf_transcoding_error::truncated_utf8_sequence:
    return "truncated_utf8_sequence";
  case utf_transcoding_error::unpaired_high_surrogate:
    return "unpaired_high_surrogate";
  case utf_transcoding_error::unpaired_low_surrogate:
    return "unpaired_low_surrogate";
  case utf_transcoding_error::unexpected_utf8_continuation_byte:
    return "unexpected_utf8_continuation_byte";
  case utf_transcoding_error::overlong:
    return "overlong";
  case utf_transcoding_error::encoded_surrogate:
    return "encoded_surrogate";
  case utf_transcoding_error::out_of_range:
    return "out_of_range";
  case utf_transcoding_error::invalid_utf8_leading_byte:
    return "invalid_utf8_leading_byte";
  }
  std::unreachable();
}

template <typename FromChar, typename ToChar>
std::basic_string<ToChar> transcode_or_throw(std::basic_string_view<FromChar> input) {
  std::basic_string<ToChar> result;
  auto view = input | to_utf_or_error<ToChar>;
  for (auto it = view.begin(), end = view.end(); it != end; ++it) {
    if ((*it).has_value()) {
      result.push_back(**it);
    } else {
      throw std::runtime_error("error at position " +
                               std::to_string(it.base() - input.begin()) + ": " +
                               enum_to_string((*it).error()));
    }
  }
  return result;
}

#if 0
// Deliberately broken by double-transcode optimization in the CPO
template <typename I, typename O>
using transcode_result = std::ranges::in_out_result<I, O>;

template <std::input_iterator I, std::sentinel_for<I> S, std::output_iterator<char8_t> O>
transcode_result<I, O> transcode_to_utf32(I first, S last, O out) {
  auto r = std::ranges::subrange(first, last) | to_utf32;

  auto copy_result = std::ranges::copy(r, out);

  return transcode_result<I, O>{copy_result.in.base(), copy_result.out};
}

bool transcode_to_utf32_test() {
  std::u8string_view char8_string{u8"\xf0\x9f\x95\xb4\xef\xbf\xbd"};
  auto utf16_transcoding_view{char8_string | to_utf16};
  std::u32string char32_string{};
  auto transcode_result{transcode_to_utf32(utf16_transcoding_view.begin(),
                                           utf16_transcoding_view.end(),
                                           std::back_insert_iterator{char32_string})};
  auto expected_in_it{utf16_transcoding_view.begin()};
  std::ranges::advance(expected_in_it, 3);
  if (expected_in_it != transcode_result.in) {
    return false;
  }
  return true;
}
#endif

#ifndef _MSC_VER
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

bool change_playing_card_suit_test() {
  std::u8string_view const spades = u8"🂡🂢🂣🂤🂥🂦🂧🂨🂩🂪🂫🂭🂮";
  std::u8string const hearts =
    spades |
    to_utf32 |
    std::views::transform(std::bind_back(change_playing_card_suit, suit::hearts)) |
    to_utf8 |
    std::ranges::to<std::u8string>();
  if (hearts != u8"🂱🂲🂳🂴🂵🂶🂷🂸🂹🂺🂻🂽🂾") {
    return false;
  }
  return true;
}
#endif

#ifdef __cpp_lib_containers_ranges
static_assert(std::string{std::from_range, "Brubeck" | std::views::take(5)} == "Brube");
// static_assert(std::string{std::from_range, "Dave" | std::views::take(5)} == "Dave"); // fails
using namespace std::string_view_literals;
static_assert(std::string{std::from_range, "Dave" | std::views::take(5)} == "Dave\0"sv); // passes
static_assert(std::is_same_v<std::remove_reference_t<decltype("foo")>, const char[4]>);
static_assert(std::ranges::equal("foo", std::array{'f', 'o', 'o', '\0'}));
constexpr std::string take_five_a(char const* long_string) {
  std::string_view const long_string_view = long_string; // read all of long_string!
  return std::string{std::from_range, long_string_view | std::views::take(5)};
}
constexpr std::string take_five_b(char const* long_string) {
  std::ranges::subrange const long_string_range(long_string, null_sentinel); // lazy!
  return std::string{std::from_range, long_string_range | std::views::take(5)};
}
constexpr std::string take_five_c(char const* long_string) {
  return std::string{std::from_range, null_term(long_string) | std::views::take(5)};
}
static_assert(take_five_a("Dave") == "Dave"sv); // passes
static_assert(take_five_a("Brubeck") == "Brube"sv); // passes
static_assert(take_five_b("Dave") == "Dave"sv); // passes
static_assert(take_five_b("Brubeck") == "Brube"sv); // passes
static_assert(take_five_c("Dave") == "Dave"sv); // passes
static_assert(take_five_c("Brubeck") == "Brube"sv); // passes
#endif

#ifndef _MSC_VER
static_assert((u8"\xf0\x9f\x99\x82"sv | to_utf32 | std::ranges::to<std::u32string>()) == U"\x0001F642");
static_assert((u8"\xf0\x9f\x99\x82"sv | std::views::take(3) | to_utf32 | std::ranges::to<std::u32string>()) == U"�");
static_assert(
  *(u8"\xf0\x9f\x99\x82"sv | std::views::take(3) | to_utf32_or_error).begin() ==
  std::unexpected{utf_transcoding_error::truncated_utf8_sequence});
#endif

#ifndef _MSC_VER
bool basis_operation() {
  std::u8string invalid_utf8{u8"\xf0\x9f\x99\x82\xf0\x9f\x99"};
  auto to_utf8_1{invalid_utf8 | to_utf8 | std::ranges::to<std::u8string>()};
  auto to_utf8_2{
    invalid_utf8
    | to_utf8_or_error
    | std::views::transform(
        [](std::expected<char8_t, utf_transcoding_error> c)
          -> detail::fake_inplace_vector<char8_t, 3>
        {
          if (c.has_value()) {
            return {c.value()};
          } else {
            // U+FFFD
            return {u8'\xEF', u8'\xBF', u8'\xBD'};
          }
        })
    | std::views::join
    | std::ranges::to<std::u8string>()};
  return to_utf8_1 == to_utf8_2;
}

static_assert(
  !std::ranges::equal(u8"foo"sv | to_utf32, std::array{U'f', U'o', U'o', U'\0'}));
static_assert(
  std::ranges::equal(u8"foo"sv | to_utf32, std::array{U'f', U'o', U'o'}));
#endif

template <typename FromCharT, typename ToCharT>
std::basic_string<ToCharT> transcode_to(std::basic_string<FromCharT> const& input) {
  return input | to_utf<ToCharT> | std::ranges::to<std::basic_string<ToCharT>>();
}

#if __cpp_lib_ranges_chunk >= 202202L
std::u8string parse_message_subset(
    std::span<std::byte> message, std::size_t offset, std::size_t length) {
  return std::span{message.begin() + offset, message.begin() + offset + length}
         | std::views::chunk(2)
         | std::views::transform(
             [](const auto chunk) {
               std::array<std::byte, 2> a{};
               std::ranges::copy(chunk, a.begin());
               return std::bit_cast<std::uint16_t>(a);
             })
         | from_big_endian
         | as_char16_t
         | to_utf8
         | std::ranges::to<std::u8string>();
}
#endif

// Byte-oriented encoding-scheme <-> encoding-form pipelines built only from the
// facilities in P4030 (endian views) and P2728 (code unit casting views), as
// discussed in the SG16 thread about P4030. std::views::concat is C++26.
#if defined(__cpp_lib_ranges_concat) && __cpp_lib_ranges_chunk >= 202202L
// Bytes (UTF-32 encoding scheme, optionally BOM-prefixed) -> encoding form.
// Design decisions: pad a trailing partial code unit with zero bytes; if no BOM
// is present assume little-endian; keep the BOM in the output.
//
// The two endian adaptors are distinct types, so the conditional expression must
// pick between two already-materialized results (both std::u32string), not
// between the two adaptors. The byte stream is modelled as std::byte (matching
// parse_message_subset above); a std::array<char, 4> staging buffer would reject
// a std::byte input range, since std::byte does not convert to char.
std::u32string parse_utf32_bytes_with_bom(std::ranges::forward_range auto bytes) {
  auto u32s =
    bytes
    | std::views::chunk(4)
    | std::views::transform(
        [](auto chunk) {
          std::array<std::byte, 4> a{};
          std::ranges::copy(chunk, a.begin());
          return std::bit_cast<std::uint32_t>(a);
        });
  std::endian const bom_endianness =
    [](std::ranges::forward_range auto values) {
      if (std::ranges::empty(values)) {
        return std::endian::little;
      }
      constexpr auto big_endian_bom{std::bit_cast<std::uint32_t>(std::array<std::byte, 4>{
          std::byte{0x00}, std::byte{0x00}, std::byte{0xFE}, std::byte{0xFF}})};
      return *std::ranges::begin(values) == big_endian_bom
             ? std::endian::big : std::endian::little;
    }(u32s);
  return bom_endianness == std::endian::little
         ? u32s | from_little_endian | std::ranges::to<std::u32string>()
         : u32s | from_big_endian | std::ranges::to<std::u32string>();
}

// Encoding form -> bytes (UTF-32LE encoding scheme with a leading BOM).
std::vector<std::byte> to_utf32le_bytes_with_bom(std::u32string utf32) {
  return std::views::concat(std::views::single(U'\xFEFF'), utf32)
    | std::views::transform(
        [](char32_t const c) {
          return static_cast<std::uint32_t>(c);
        })
    | to_little_endian
    | std::views::transform(
        [](std::uint32_t const x) {
          return std::bit_cast<std::array<std::byte, 4>>(x);
        })
    | std::views::join
    | std::ranges::to<std::vector>();
}
#endif

#if __cpp_lib_ranges_chunk_by >= 202202L
template <typename T>
constexpr bool is_continuation(T c) {
  if constexpr (std::is_same_v<decltype(c), char8_t>) {
    return (c & 0xC0) == 0x80;
  } else if constexpr (std::is_same_v<decltype(c), char16_t>) {
    return c >= 0xDC00 && c <= 0xDFFF;
  } else {
    return false;
  }
}

template <typename FromType, typename ToType, std::size_t N>
constexpr detail::fake_inplace_vector<ToType, N> transcode_truncating_correctly(
    std::basic_string_view<FromType> input) {
  detail::fake_inplace_vector<ToType, N> output;
  for (auto code_point_view : input
       | to_utf<ToType>
       | std::views::chunk_by([](auto, auto b) { return is_continuation(b); })) {
    if (static_cast<std::size_t>(std::ranges::distance(code_point_view))
        > output.max_size() - output.size())
      break;
    std::ranges::copy(code_point_view, std::back_insert_iterator{output});
  }
  return output;
}
#endif

#if __cpp_lib_ranges_as_input >= 202502L
void print_utf8_code_points_and_code_units(std::ranges::range auto input) {
  auto print_code_point{
    [](char32_t code_point, auto code_unit_range) {
    std::println(
      "{:#x} = {::#x}", static_cast<std::uint32_t>(code_point),
      code_unit_range
      | std::views::transform([](char8_t c) { return (std::uint8_t)c; }));
    }};
  auto code_points = input
                     | std::ranges::to<std::u8string>()
                     | beman::utf_view::to_utf32;
  for (auto it = code_points.begin(); it != code_points.end(); ++it) {
    print_code_point(*it, std::ranges::subrange(it.base(), std::ranges::next(it).base()));
  }
}

constexpr bool is_utf8_continuation(char8_t c) { return (c & 0xC0) == 0x80; }

void print_utf16_and_utf8_code_units_per_code_point(std::ranges::range auto input) {
  auto print_code_point{
    [](auto u16_view, auto u8_view) {
      std::println(
        "{::#x} = {::#x}",
        u16_view | std::views::transform([](char16_t c) { return (std::uint16_t)c; }),
        u8_view | std::views::transform([](char8_t c) { return (std::uint8_t)c; }));
    }};
  auto it = input.begin();
  std::u8string code_point;
  while (it != input.end()) {
    code_point.clear();
    code_point.push_back(*it);                     // lead byte
    ++it;
    it = std::ranges::find_if(std::move(it), input.end(), [&](char8_t c) {
      if (!is_utf8_continuation(c)) return true;    // next lead: stop, don't consume
      code_point.push_back(c);                      // continuation: keep
      return false;
    });
    print_code_point(code_point | beman::utf_view::to_utf16, code_point);
  }
}

#endif

bool readme_examples() {
  using namespace std::string_view_literals;
#ifndef _MSC_VER
  std::u32string hello_world =
      u8"こんにちは世界"sv | to_utf32 | std::ranges::to<std::u32string>();
  if (hello_world != U"こんにちは世界") {
    return false;
  }
  if (sanitize(u8"\xc2") != u8"\xef\xbf\xbd") {
    return false;
  }
  if (last_nonascii("hôtel"sv).value() != U'ô') {
    return false;
  }
  if (as_char32_t_example() != u8"\xf0\x9f\x95\xb4\xef\xbf\xbd") {
    return false;
  }
  auto foo = transcode_or_throw<char8_t, char32_t>(u8"\xf0\x9f\x95\xb4\xef\xbf\xbd");
  auto bar = std::u32string{U"\x0001F574\uFFFD"};
  if (foo != bar) {
    return false;
  }
  try {
    transcode_or_throw<char8_t, char32_t>(u8"\xc3\xa9\xff");
    return false;
  } catch (std::exception const& e) {
    if (e.what() != "error at position 2: invalid_utf8_leading_byte"sv) {
      return false;
    }
  }
  if (!change_playing_card_suit_test()) {
    return false;
  }
  if (!basis_operation()) {
    return false;
  }
#else
  if (!windows_path()) {
    return false;
  }
#endif
  if (transcode_to<char8_t, char32_t>(u8"foo") != U"foo") {
    return false;
  }
#ifndef _MSC_VER // TODO: figure out why this test fails on MSVC
#if __cpp_lib_ranges_chunk >= 202202L
  std::array<std::byte, 6> message{
    std::byte{0x12}, std::byte{0xD8}, std::byte{0x3D}, std::byte{0xDE}, std::byte{0x42},
    std::byte{0x34}};
  if (!std::ranges::equal(u8"\xf0\x9f\x99\x82"sv, parse_message_subset(message, 1, 4))) {
    return false;
  }
#if defined(__cpp_lib_ranges_concat) && __cpp_lib_ranges_chunk >= 202202L
  {
    std::u32string const original = U"Aé\U0001F642"; // "Aé🙂"
    // Round-trip: encoding form -> UTF-32LE bytes (with BOM) -> encoding form.
    // parse keeps the BOM, so the result is U+FEFF followed by the original.
    std::vector<std::byte> const le_bytes = to_utf32le_bytes_with_bom(original);
    // First four bytes are the little-endian BOM: FF FE 00 00.
    std::array<std::byte, 4> const le_bom{
      std::byte{0xFF}, std::byte{0xFE}, std::byte{0x00}, std::byte{0x00}};
    if (!std::ranges::equal(le_bytes | std::views::take(4), le_bom)) {
      return false;
    }
    if (le_bytes.size() != 4 * (original.size() + 1)) {
      return false;
    }
    if (parse_utf32_bytes_with_bom(le_bytes) != std::u32string{U'\uFEFF'} + original) {
      return false;
    }
    // A big-endian BOM (00 00 FE FF) is detected; the BOM is kept in the output.
    std::vector<std::byte> be_bytes{
      std::byte{0x00}, std::byte{0x00}, std::byte{0xFE}, std::byte{0xFF}, // BOM
      std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x41}}; // 'A'
    if (parse_utf32_bytes_with_bom(be_bytes) != (std::u32string{U'\uFEFF'} + U"A")) {
      return false;
    }
    // No BOM: assume little-endian, keep all code points.
    std::vector<std::byte> no_bom{
      std::byte{0x41}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}}; // 'A' LE
    if (parse_utf32_bytes_with_bom(no_bom) != U"A") {
      return false;
    }
  }
#endif
  {
    auto result = transcode_truncating_correctly<char8_t, char16_t, 5>(u8"😀abc"sv);
    if (result.size() != 5) {
      return false;
    }
    auto result2 = transcode_truncating_correctly<char8_t, char16_t, 4>(u8"😀abc"sv);
    if (result2.size() != 4) {
      return false;
    }
    auto result3 = transcode_truncating_correctly<char8_t, char16_t, 2>(u8"😀abc"sv);
    if (result3.size() != 2) {
      return false;
    }
    auto result4 = transcode_truncating_correctly<char8_t, char16_t, 1>(u8"😀abc"sv);
    if (result4.size() != 0) {
      return false;
    }
  }
#endif
#if __cpp_lib_ranges_as_input >= 202502L
  print_utf8_code_points_and_code_units(u8"AΩ€😀b"sv | std::views::as_input);
  print_utf16_and_utf8_code_units_per_code_point(u8"AΩ€😀b"sv | std::views::as_input);
#endif
  {
    std::u16string zamin = u"𒀭𒎏𒄈𒋢𒍠𒊩";
    auto view = zamin | to_utf32;
    auto begin = view.begin();
    ++begin;
    auto ningirsuBegin = begin.base();
    std::ranges::advance(begin, 3);
    auto ningirsuEnd = begin.base();
    zamin.replace(ningirsuBegin, ningirsuEnd, u"𒊺𒉀");
    if (!std::ranges::equal(zamin, u"𒀭𒊺𒉀𒍠𒊩"sv)) {
      return false;
    }
  }
#endif
  return true;
}

} // namespace beman::utf_view::examples

void windows_function(std::ranges::view auto) {

}

void foo(std::ranges::view auto v) {
  windows_function(v | beman::utf_view::to_utf16);
}

int main(int, char const* argv[]) {
  foo(beman::utf_view::null_term(argv[1]) | beman::utf_view::as_char8_t | beman::utf_view::to_utf32);
  return beman::utf_view::examples::readme_examples() ? 0 : 1;
}
