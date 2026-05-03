#include <beman/utf_view/config.hpp>
#include <beman/utf_view/null_term.hpp>
#if BEMAN_UTF_VIEW_USE_MODULES()
import std;
#else
#include <print>
#endif

extern char** environ;

#if __cpp_lib_format_ranges >= 202207
int main([[maybe_unused]] int argc, char** argv) {
  std::println("argv: {}", beman::utf_view::null_term(argv));
  std::println("environ: {}", beman::utf_view::null_term(environ));
}
#else
int main() {}
#endif
