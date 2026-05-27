#include <beman/utf_view/config.hpp>
#include <beman/utf_view/code_unit_view.hpp>
#include <beman/utf_view/to_utf_view.hpp>
#if BEMAN_UTF_VIEW_USE_MODULES()
import std;
#else
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#endif

// Usage: printf 'Dusíková\n42' | ./player_name
//
// Reads a player name (code points until newline), then a score, from stdin.
//
// This works with a scalar to_utf32 iterator: take_while's sentinel
// peeks via *it without consuming, so bytes after the newline stay in
// the streambuf for cin >> score.
//
// A SIMD-backed to_utf32 would speculatively pull 16+ bytes from the
// istreambuf_iterator to fill a register, consuming the score bytes
// before take_while ever gets to check the predicate.

int main() {
  std::ranges::subrange input(
      std::istreambuf_iterator<char>(std::cin),
      std::istreambuf_iterator<char>{});

  auto player_name = input
      | beman::utf_view::as_char8_t
      | beman::utf_view::to_utf32
      | std::views::take_while([](char32_t c) { return c != U'\n'; })
      | std::ranges::to<std::u32string>();

  int score{};
  std::cin >> score;

  std::print("Player: {}\nScore: {}\n",
      player_name
      | beman::utf_view::to_utf8
      | beman::utf_view::as_char
      | std::ranges::to<std::string>(),
      score);
}
