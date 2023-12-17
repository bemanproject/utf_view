// SPDX-License-Identifier: GPL-3.0-only

#include <UtfView/detail/concepts.hpp>
#include <tests/std_archetypes/iterator.hpp>

namespace utfview::tests {

static_assert(exposition_only_code_unit_to<char8_t>);
static_assert(exposition_only_code_unit_to<char16_t>);
static_assert(exposition_only_code_unit_to<char32_t>);
static_assert(!exposition_only_code_unit_to<wchar_t>);
static_assert(!exposition_only_code_unit_to<char>);
static_assert(!exposition_only_code_unit_to<int>);

static_assert(exposition_only_code_unit_from<char8_t>);
static_assert(exposition_only_code_unit_from<char16_t>);
static_assert(exposition_only_code_unit_from<char32_t>);
static_assert(exposition_only_code_unit_from<wchar_t>);
static_assert(exposition_only_code_unit_from<char>);
static_assert(!exposition_only_code_unit_from<int>);

// todo: utf_range

} // namespace utfview::detail::tests
