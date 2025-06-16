#include <beman/utf_view/null_term.hpp>
#include <print>

extern char** environ;

int main(int argc, char** argv) {
#if __cpp_lib_format_ranges >= 202207
  std::println("argv: {}", beman::utf_view::null_term(argv));
  std::println("environ: {}", beman::utf_view::null_term(environ));
#endif
}
