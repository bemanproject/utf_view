// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <utf_view/null_term.hpp>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <tests/std_archetypes/iterator.hpp>
#include <utility>

namespace utf_view::tests {

static_assert(std::default_initializable<
              std::iter_value_t<std_archetypes::input_iterator_archetype>>);
static_assert(std::equality_comparable_with<
              std::iter_reference_t<std_archetypes::input_iterator_archetype>,
              std::iter_value_t<std_archetypes::input_iterator_archetype>>);
static_assert(
    std::sentinel_for<null_sentinel_t, std_archetypes::input_iterator_archetype>);
static_assert(std::default_initializable<
              std::iter_value_t<std_archetypes::forward_iterator_archetype>>);
static_assert(std::equality_comparable_with<
              std::iter_reference_t<std_archetypes::forward_iterator_archetype>,
              std::iter_value_t<std_archetypes::forward_iterator_archetype>>);
static_assert(
    std::sentinel_for<null_sentinel_t, std_archetypes::forward_iterator_archetype>);

constexpr bool null_sentinel_input_iterator_test() {
  int total = 0;
  std_archetypes::input_iterator_archetype it{254};
  for (auto x : null_term(std::move(it))) {
    total += x.x;
  }
  return total == 254 + 255;
}

constexpr bool null_sentinel_forward_iterator_test() {
  int total = 0;
  std_archetypes::forward_iterator_archetype it{254};
  for (auto x : null_term(it)) {
    total += x.x;
  }
  return total == 254 + 255;
}

static_assert(null_sentinel_input_iterator_test());
static_assert(null_sentinel_forward_iterator_test());

} // namespace utf_view::tests
