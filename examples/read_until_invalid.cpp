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

// Usage: printf 'Dusíková\xff rest' | ./read_until_invalid
//
// Reads valid UTF-8 from stdin until the first encoding error, then
// reads the remaining bytes separately.
//
// With a scalar to_utf8_or_error iterator this works: the validator
// reads only the bytes it needs, so everything past the first invalid
// byte stays in the streambuf.
//
// A SIMD-backed implementation would speculatively pull 16+ bytes
// from the istreambuf_iterator to fill a vector register, consuming
// bytes past the error before take_while can stop the pipeline.

std::string read_until_invalid(std::ranges::input_range auto&& input) {
  return input
      | beman::utf_view::as_char8_t
      | beman::utf_view::to_utf8_or_error
      | std::views::take_while(&std::expected<char8_t,
            beman::utf_view::utf_transcoding_error>::has_value)
      | std::views::transform([](auto c) { return static_cast<char>(c.value()); })
      | std::ranges::to<std::string>();
}

int main() {
  std::ranges::subrange input(
      std::istreambuf_iterator<char>(std::cin),
      std::istreambuf_iterator<char>{});

  auto valid_prefix = read_until_invalid(input);

  auto rest = std::ranges::subrange(
      std::istreambuf_iterator<char>(std::cin),
      std::istreambuf_iterator<char>{})
      | std::ranges::to<std::string>();

  std::print("Valid: {}\nRest:  {}\n", valid_prefix, rest);
}
