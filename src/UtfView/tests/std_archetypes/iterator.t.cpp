// SPDX-License-Identifier: GPL-3.0-only

#include <iterator>
#include <tests/std_archetypes/iterator.hpp>

namespace utfview::tests::std_archetypes {

static_assert(std::input_iterator<input_iterator_archetype>);
static_assert(std::input_iterator<basic_input_iterator_archetype<int>>);
static_assert(std::forward_iterator<forward_iterator_archetype>);
static_assert(std::forward_iterator<basic_forward_iterator_archetype<int>>);
static_assert(std::bidirectional_iterator<bidirectional_iterator_archetype>);
static_assert(std::bidirectional_iterator<basic_bidirectional_iterator_archetype<int>>);

} // namespace utfview::tests::std_archetypes
