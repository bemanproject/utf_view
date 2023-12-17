// SPDX-License-Identifier: GPL-3.0-only

#include <tests/std_archetypes/exposition_only.hpp>

namespace utfview::tests::std_archetypes {

#if defined(_GLIBCXX_VERSION_INCLUDED) || defined(_LIBCPP_VERSION)
static_assert(__boolean_testable<boolean_testable_archetype1>);
static_assert(__boolean_testable<boolean_testable_archetype2>);
#endif

} // namespace utfview::tests::std_archetypes
