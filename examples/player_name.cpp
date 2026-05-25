#include <beman/utf_view/config.hpp>
#include <beman/utf_view/code_unit_view.hpp>
#include <beman/utf_view/to_utf_view.hpp>
#include <termios.h>
#include <unistd.h>
#if BEMAN_UTF_VIEW_USE_MODULES()
import std;
#else
#include <iostream>
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
  // get 5 code points of input
  return input_view
         | beman::utf_view::as_char8_t
         | beman::utf_view::to_utf32
         | std::views::take(5)
         | std::ranges::to<std::u32string>();
}

int main() {
  raw_mode rm;
  get_player_name();
}
