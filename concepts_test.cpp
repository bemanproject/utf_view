export module p2728:concepts_test;

import std_archetypes;
import :concepts;

namespace p2728 {

  static_assert(EOcode_unitOE<char8_t>);
  static_assert(EOcode_unitOE<char16_t>);
  static_assert(EOcode_unitOE<char32_t>);
#ifdef WCHAR_T_IS_CODE_UNIT
  static_assert(EOcode_unitOE<wchar_t>);
#ifdef CHAR_IS_CODE_UNIT
  static_assert(EOcode_unitOE<char>);
#else
  static_assert(!EOcode_unitOE<char>);
#endif
#else
  static_assert(!EOcode_unitOE<wchar_t>);
#endif
  static_assert(!EOcode_unitOE<int>);

  static_assert(
    EOutf_iterOE<
      std_archetypes::basic_input_iterator_archetype<
        char8_t, char8_t, char8_t const&>>);
  static_assert(
    EOutf_iterOE<
      std_archetypes::basic_input_iterator_archetype<
        char16_t, char16_t, char16_t const&>>);
  static_assert(
    EOutf_iterOE<
      std_archetypes::basic_input_iterator_archetype<
        char32_t, char32_t, char32_t const&>>);
#ifdef WCHAR_T_IS_CODE_UNIT
  static_assert(
    EOutf_iterOE<
      std_archetypes::basic_input_iterator_archetype<
        wchar_t, wchar_t, wchar_t const&>>);
#ifdef CHAR_IS_CODE_UNIT
  static_assert(
    EOutf_iterOE<
      std_archetypes::basic_input_iterator_archetype<
        char, char, char const&>>);
#else
  static_assert(
    !EOutf_iterOE<
      std_archetypes::basic_input_iterator_archetype<
        char, char, char const&>>);
#endif
#else
  static_assert(
    !EOutf_iterOE<
      std_archetypes::basic_input_iterator_archetype<
        wchar_t, wchar_t, wchar_t const&>>);
#endif
  static_assert(
    !EOutf_iterOE<
      std_archetypes::basic_input_iterator_archetype<
        int, int, int const&>>);

#ifdef CHARN_T_POINTERS_ARE_RANGE_LIKE
  static_assert(EOutf_pointerOE<char8_t*>);
  static_assert(EOutf_pointerOE<char16_t*>);
  static_assert(EOutf_pointerOE<char32_t*>);
#ifdef WCHAR_T_IS_CODE_UNIT
  static_assert(EOutf_pointerOE<wchar_t*>);
#ifdef CHAR_IS_CODE_UNIT
  static_assert(EOutf_pointerOE<char*>);
#else
  static_assert(!EOutf_pointerOE<char*>);
#endif
#else
  static_assert(!EOutf_pointerOE<wchar_t*>);
#endif
  static_assert(!EOutf_pointerOE<int*>);
#endif

// todo: utf-range tests

} // namespace p2728
