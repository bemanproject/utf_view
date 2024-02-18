module;

#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>

export module p2728:concepts;
import null_term;

namespace p2728 {

  using namespace std;

  template <class T>
    concept EOcode_unitOE = same_as<T, char8_t> || same_as<T, char16_t> ||
                        same_as<T, char32_t>
#ifdef WCHAR_T_IS_CODE_UNIT
                        || same_as<T, wchar_t>
#ifdef CHAR_IS_CODE_UNIT
                        || same_as<T, char>
#endif
#endif
        ;

  template<class T>
    concept EOutf_iterOE = input_iterator<T> && EOcode_unitOE<iter_value_t<T>>;
#ifdef CHARN_T_POINTERS_ARE_RANGE_LIKE
  template<class T>
    concept EOutf_pointerOE = is_pointer_v<T> && EOcode_unitOE<iter_value_t<T>>;
#endif
  template<class T>
    concept EOutf_rangeOE = ranges::input_range<T> && EOcode_unitOE<ranges::range_value_t<T>>;

#ifdef CHARN_T_POINTERS_ARE_RANGE_LIKE
  template<class T>
    concept EOutf-range-likeOE =
      EOutf_rangeOE<remove_reference_t<T>> || EOutf_pointerOE<remove_reference_t<T>>;
#endif

} // namespace p2728
