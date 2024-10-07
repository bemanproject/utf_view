// SPDX-License-Identifier: GPL-3.0-only

#include <UtfView/code_unit_view.hpp>
#include <UtfView/detail/constexpr_unless_msvc.hpp>
#include <ranges>
#include <string_view>
#include <tests/framework.hpp>

namespace utfview::tests {

// TODO: Comprehensive testing for `code_unit_view`

constexpr bool smoke_test() {
  std::string_view foo{"foo"};
  auto bar{foo | as_char8_t};
  static_assert(std::ranges::borrowed_range<decltype(foo)>);
  auto baz{bar | std::ranges::to<std::u8string>()};
  if (baz != std::u8string_view{u8"foo"}) {
    return false;
  }
  return true;
}

CONSTEXPR_UNLESS_MSVC bool code_unit_view_test() {
  if (!smoke_test()) {
    return false;
  }
  return true;
}

#ifndef _MSC_VER
static_assert(code_unit_view_test());
#endif

static auto const init{[] {
  framework::tests().insert({"code_unit_view_test", &code_unit_view_test});
  struct {
  } result{};
  return result;
}()};

} // namespace utfview::tests
