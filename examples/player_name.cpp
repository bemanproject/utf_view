#include <beman/utf_view/config.hpp>
#include <beman/utf_view/code_unit_view.hpp>
#include <beman/utf_view/to_utf_view.hpp>
#include <termios.h>
#include <unistd.h>
#include <cassert>
#if BEMAN_UTF_VIEW_USE_MODULES()
import std;
#else
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#endif

struct raw_mode {
    termios old{};
    raw_mode() {
        tcgetattr(STDIN_FILENO, &old);
        termios n = old;
        n.c_lflag &= ~(ICANON | ECHO);
        n.c_cc[VMIN]  = 1;   // return after 1 byte
        n.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &n);
    }
    ~raw_mode() { tcsetattr(STDIN_FILENO, TCSANOW, &old); }
};

std::u32string get_player_name() {
  std::ranges::subrange input_view(
      std::istreambuf_iterator<char>(std::cin),
      std::istreambuf_iterator<char>{});
  auto utf32_view = input_view
                    | beman::utf_view::as_char8_t
                    | beman::utf_view::to_utf32;
  std::u32string result;
  auto it = utf32_view.begin();
  result.push_back(*it);
  for (int i = 1; i < 5; ++i)
    result.push_back(*++it);
  return result;
}

int main() {
  std::print("Enter your 5-character player name:");
  std::u32string const player_name{
    []{
      raw_mode rm;
      return get_player_name();
    }()};
  assert(player_name.size() == 5);
  std::print(
      "\nWelcome, {}\n",
      player_name
      | beman::utf_view::to_utf8
      | beman::utf_view::as_char
      | std::ranges::to<std::string>());
}
