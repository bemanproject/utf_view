// SPDX-License-Identifier: GPL-3.0-only

#include <UtfView/null_term.hpp>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <tests/std_archetypes/iterator.hpp>
#include <utility>

namespace utfview::tests {

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

} // namespace utfview::tests
