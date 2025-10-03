// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <beman/utf_view/detail/concepts.hpp>
#include <std_archetypes/iterator.hpp>

namespace beman::utf_view::tests {

static_assert(exposition_only_code_unit<char8_t>);
static_assert(exposition_only_code_unit<char16_t>);
static_assert(exposition_only_code_unit<char32_t>);
static_assert(!exposition_only_code_unit<wchar_t>);
static_assert(!exposition_only_code_unit<char>);
static_assert(!exposition_only_code_unit<int>);

// todo: utf_range

} // namespace beman::utf_view::detail::tests
