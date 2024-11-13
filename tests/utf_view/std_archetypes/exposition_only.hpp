// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef UTF_VIEW_TESTS_STD_ARCHETYPES_EXPOSITION_ONLY_HPP
#define UTF_VIEW_TESTS_STD_ARCHETYPES_EXPOSITION_ONLY_HPP

namespace utf_view::tests::std_archetypes {

class boolean_testable_archetype2;

class boolean_testable_archetype1 {
public:
  constexpr explicit boolean_testable_archetype1(bool inner_in)
      : inner_{inner_in} { }
  constexpr operator bool() const noexcept {
    return inner_;
  }
  constexpr boolean_testable_archetype2 operator!() const noexcept;

private:
  bool inner_;
};

class boolean_testable_archetype2 {
public:
  constexpr explicit boolean_testable_archetype2(bool inner_in)
      : inner_{inner_in} { }
  constexpr operator bool() const noexcept {
    return inner_;
  }
  constexpr boolean_testable_archetype1 operator!() const noexcept {
    return boolean_testable_archetype1{!inner_};
  }

private:
  bool inner_;
};

constexpr boolean_testable_archetype2 boolean_testable_archetype1::operator!()
    const noexcept {
  return boolean_testable_archetype2{inner_};
}

} // namespace utf_view::tests::std_archetypes

#endif // UTF_VIEW_TESTS_STD_ARCHETYPES_EXPOSITION_ONLY_HPP
