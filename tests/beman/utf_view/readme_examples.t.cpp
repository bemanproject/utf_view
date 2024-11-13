// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <framework.hpp>
#include <beman/utf_view/code_unit_view.hpp>
#include <beman/utf_view/null_term.hpp>
#include <beman/utf_view/to_utf_view.hpp>
#include <filesystem>
#include <iterator>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace beman::utf_view::tests {

template <typename CharT>
std::basic_string<CharT> sanitize(CharT const* str) {
  return null_term(str) | to_utf<CharT> | std::ranges::to<std::basic_string<CharT>>();
}

std::optional<char32_t> last_nonascii(std::ranges::view auto str) {
  for (auto c : str | to_utf32 | std::views::reverse |
           std::views::filter([](char32_t c) { return c > 0x7f; }) |
           std::views::take(1)) {
    return c;
  }
  return std::nullopt;
}

#ifdef _MSC_VER
bool windows_path() {
  std::vector<int> path_as_ints = {U'C', U':', U'\x00010000'};
  std::filesystem::path path =
      path_as_ints | as_char32_t | std::ranges::to<std::u32string>();
  auto const& native_path = path.native();
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

std::string enum_to_string(transcoding_error ec) {
  switch (ec) {
  case transcoding_error::truncated_utf8_sequence:
    return "truncated_utf8_sequence";
  case transcoding_error::unpaired_high_surrogate:
    return "unpaired_high_surrogate";
  case transcoding_error::unpaired_low_surrogate:
    return "unpaired_low_surrogate";
  case transcoding_error::unexpected_utf8_continuation_byte:
    return "unexpected_utf8_continuation_byte";
  case transcoding_error::overlong:
    return "overlong";
  case transcoding_error::encoded_surrogate:
    return "encoded_surrogate";
  case transcoding_error::out_of_range:
    return "out_of_range";
  case transcoding_error::invalid_utf8_leading_byte:
    return "invalid_utf8_leading_byte";
  }
  std::unreachable();
}

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
  to_utf16_view<std::u8string_view> utf16_transcoding_view{char8_string};
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

bool readme_examples() {
  using namespace std::string_view_literals;
#ifndef _MSC_VER
  std::u32string hello_world =
      u8"こんにちは世界" | to_utf32 | std::ranges::to<std::u32string>();
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
  if (!transcode_to_utf32_test()) {
    return false;
  }
  if (!change_playing_card_suit_test()) {
    return false;
  }
#else
  if (!windows_path()) {
    return false;
  }
#endif
  return true;
}

static auto const init{[] {
  framework::tests().insert({"readme_examples", &readme_examples});
  struct {
  } result{};
  return result;
}()};

} // namespace beman::utf_view::tests
