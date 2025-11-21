// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <std_archetypes/iterator.hpp>
#include <iterator>

namespace beman::utf_view::tests::std_archetypes {

static_assert(std::input_iterator<input_iterator_archetype>);
static_assert(std::input_iterator<basic_input_iterator_archetype<int>>);
static_assert(std::forward_iterator<forward_iterator_archetype>);
static_assert(std::forward_iterator<basic_forward_iterator_archetype<int>>);
static_assert(std::bidirectional_iterator<bidirectional_iterator_archetype>);
static_assert(std::bidirectional_iterator<basic_bidirectional_iterator_archetype<int>>);

} // namespace beman::utf_view::tests::std_archetypes
