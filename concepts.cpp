module;

#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>

export module p2728:concepts;
import null_term;

namespace p2728 {

  using namespace std;

  template<class T>
  concept EOcode_unitOE = same_as<remove_cv_t<T>, char8_t> || same_as<remove_cv_t<T>, char16_t> || same_as<remove_cv_t<T>, char32_t>;

  template<class T>
  concept EOutf_iterOE = input_iterator<T> && EOcode_unitOE<iter_value_t<T>>;
  template<class T>
  concept EOutf_pointerOE = is_pointer_v<T> && EOcode_unitOE<iter_value_t<T>>;
  template<class T>
  concept EOutf_rangeOE = ranges::input_range<T> && EOcode_unitOE<ranges::range_value_t<T>>;

  template<class T>
  concept EOutf_range_likeOE = EOutf_rangeOE<remove_reference_t<T>> || EOutf_pointerOE<remove_reference_t<T>>;

} // namespace p2728
