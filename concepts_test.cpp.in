module p2728:concepts_test;

import std_archetypes;
import :concepts;

namespace p2728 {

  static_assert(code_unit<char8_t>);
  static_assert(code_unit<char16_t>);
  static_assert(code_unit<char32_t>);
#ifdef WCHAR_T_IS_CODE_UNIT
  static_assert(code_unit<wchar_t>);
#ifdef CHAR_IS_CODE_UNIT
  static_assert(code_unit<char>);
#else
  static_assert(!code_unit<char>);
#endif
#else
  static_assert(!code_unit<wchar_t>);
#endif
  static_assert(!code_unit<int>);

  static_assert(utf_iter<std_archetypes::basic_input_iterator_archetype<char8_t, char8_t const&,
                                                                       char8_t>>);
  static_assert(utf_iter<std_archetypes::basic_input_iterator_archetype<char16_t, char16_t const&,
                                                                       char16_t>>);
  static_assert(utf_iter<std_archetypes::basic_input_iterator_archetype<char32_t, char32_t const&,
                                                                       char32_t>>);
#ifdef WCHAR_T_IS_CODE_UNIT
  static_assert(utf_iter<std_archetypes::basic_input_iterator_archetype<wchar_t, wchar_t const&,
                                                                       wchar_t>>);
#ifdef CHAR_IS_CODE_UNIT
  static_assert(utf_iter<std_archetypes::basic_input_iterator_archetype<char, char const&,
                                                                       char>>);
#else
  static_assert(!utf_iter<std_archetypes::basic_input_iterator_archetype<char, char const&,
                                                                        char>>);
#endif
#else
  static_assert(!utf_iter<std_archetypes::basic_input_iterator_archetype<wchar_t, wchar_t const&,
                                                                       wchar_t>>);
#endif
  static_assert(!utf_iter<std_archetypes::basic_input_iterator_archetype<int, int const&, int>>);

#ifdef CHARN_T_POINTERS_ARE_RANGE_LIKE
  static_assert(utf_pointer<char8_t*>);
  static_assert(utf_pointer<char16_t*>);
  static_assert(utf_pointer<char32_t*>);
#ifdef WCHAR_T_IS_CODE_UNIT
  static_assert(utf_pointer<wchar_t*>);
#ifdef CHAR_IS_CODE_UNIT
  static_assert(utf_pointer<char*>);
#else
  static_assert(!utf_pointer<char*>);
#endif
#else
  static_assert(!utf_pointer<wchar_t*>);
#endif
  static_assert(!utf_pointer<int*>);
#endif

} // namespace p2728
