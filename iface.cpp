module;

#include <ranges>
#include <type_traits>

export module p2728;

import null_term;

namespace p2728 {

  using namespace std;

  export {

#ifdef USE_FORMAT_ENUMERATION
  enum class format { utf8 = 1, utf16 = 2, utf32 = 4 };
#endif

  } // export

#ifdef WCHAR_T_IS_CODE_UNIT
  inline constexpr format wchar_t_format = static_cast<format>(sizeof(wchar_t));
#endif

  export {

#ifdef USE_FORMAT_ENUMERATION
  template<class T, format F>
    concept code_unit = (same_as<T, char8_t> && F == format::utf8) ||
                        (same_as<T, char16_t> && F == format::utf16) ||
                        (same_as<T, char32_t> && F == format::utf32)
#ifdef WCHAR_T_IS_CODE_UNIT
                        || (same_as<T, wchar_t> && F == wchar_t_format)
#ifdef CHAR_IS_CODE_UNIT
                        || (same_as<T, char> && F == format::utf8)
#endif
#endif
        ;
#else
  template <class T>
    concept code_unit = same_as<T, char8_t> || same_as<T, char16_t> ||
                        same_as<T, char32_t>;
#endif

#ifdef USE_FORMAT_ENUMERATION
  template<class T>
    concept utf8_code_unit = code_unit<T, format::utf8>;

  template<class T>
    concept utf16_code_unit = code_unit<T, format::utf16>;

  template<class T>
    concept utf32_code_unit = code_unit<T, format::utf32>;

  template<class T>
    concept utf_code_unit = utf8_code_unit<T> || utf16_code_unit<T> || utf32_code_unit<T>;
#endif

#ifdef USE_FORMAT_ENUMERATION
  template<class T, format F>
    concept code_unit_iter =
      input_iterator<T> && code_unit<iter_value_t<T>, F>;
  template<class T, format F>
    concept code_unit_pointer =
      is_pointer_v<T> && code_unit<iter_value_t<T>, F>;
  template<class T, format F>
    concept code_unit_range = ranges::input_range<T> &&
      code_unit<ranges::range_value_t<T>, F>;

  template<class T>
    concept utf8_iter = code_unit_iter<T, format::utf8>;
  template<class T>
    concept utf8_pointer = code_unit_pointer<T, format::utf8>;
  template<class T>
    concept utf8_range = code_unit_range<T, format::utf8>;

  template<class T>
    concept utf16_iter = code_unit_iter<T, format::utf16>;
  template<class T>
    concept utf16_pointer = code_unit_pointer<T, format::utf16>;
  template<class T>
    concept utf16_range = code_unit_range<T, format::utf16>;

  template<class T>
    concept utf32_iter = code_unit_iter<T, format::utf32>;
  template<class T>
    concept utf32_pointer = code_unit_pointer<T, format::utf32>;
  template<class T>
    concept utf32_range = code_unit_range<T, format::utf32>;

  template<class T>
    concept utf_iter = utf8_iter<T> || utf16_iter<T> || utf32_iter<T>;
  template<class T>
    concept utf_pointer = utf8_pointer<T> || utf16_pointer<T> || utf32_pointer<T>;
  template<class T>
    concept utf_range = utf8_range<T> || utf16_range<T> || utf32_range<T>;
#else
  template<class T>
    concept utf8_iter = input_iterator<T> && same_as<iter_value_t<T>, char8_t>;
  template<class T>
    concept utf8_pointer = is_pointer_v<T> && same_as<iter_value_t<T>, char8_t>;
  template<class T>
    concept utf8_range = ranges::input_range<T> && same_as<iter_value_t<T>, char8_t>;

  template<class T>
    concept utf16_iter = input_iterator<T> && same_as<iter_value_t<T>, char16_t>;
  template<class T>
    concept utf16_pointer = is_pointer_v<T> && same_as<iter_value_t<T>, char16_t>;
  template<class T>
    concept utf16_range = ranges::input_range<T> && same_as<iter_value_t<T>, char16_t>;

  template<class T>
    concept utf32_iter = input_iterator<T> && same_as<iter_value_t<T>, char32_t>;
  template<class T>
    concept utf32_pointer = is_pointer_v<T> && same_as<iter_value_t<T>, char32_t>;
  template<class T>
    concept utf32_range = ranges::input_range<T> && same_as<iter_value_t<T>, char32_t>;

  template<class T>
    concept utf_iter = utf8_iter<T> || utf16_iter<T> || utf32_iter<T>;
  template<class T>
    concept utf_pointer = utf8_pointer<T> || utf16_pointer<T> || utf32_pointer<T>;
  template<class T>
    concept utf_range = utf8_range<T> || utf16_range<T> || utf32_range<T>;
#endif

#ifdef CHARN_T_POINTERS_ARE_RANGE_LIKE
  template<class T>
    concept utf_range_like =
      utf_range<remove_reference_t<T>> || utf_pointer<remove_reference_t<T>>;
#endif

  } // export

} // namespace p2728
