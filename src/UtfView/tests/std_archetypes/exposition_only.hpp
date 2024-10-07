// SPDX-License-Identifier: GPL-3.0-only

#ifndef UTFVIEW_TESTS_STD_ARCHETYPES_EXPOSITION_ONLY_HPP
#define UTFVIEW_TESTS_STD_ARCHETYPES_EXPOSITION_ONLY_HPP

namespace utfview::tests::std_archetypes {

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

} // namespace utfview::tests::std_archetypes

#endif // UTFVIEW_TESTS_STD_ARCHETYPES_EXPOSITION_ONLY_HPP
