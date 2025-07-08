#include <beman/utf_view/null_term.hpp>
#include <print>

extern char** environ;

#if __cpp_lib_format_ranges >= 202207
int main([[maybe_unused]] int argc, char** argv) {
  std::println("argv: {}", beman::utf_view::null_term(argv));
  std::println("environ: {}", beman::utf_view::null_term(environ));
}
#else
int main() {}
#endif
