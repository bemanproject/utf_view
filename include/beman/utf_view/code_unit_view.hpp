// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef BEMAN_UTF_VIEW_CODE_UNIT_VIEW_HPP
#define BEMAN_UTF_VIEW_CODE_UNIT_VIEW_HPP

#include <beman/utf_view/detail/concepts.hpp>
#include <beman/iterator_interface/iterator_interface.hpp>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

namespace beman::utf_view {

/* PAPER: namespace std::uc { */

/* PAPER */

template <class I>
consteval auto exposition_only_iterator_to_tag() { // @*exposition only*@
  if constexpr (std::random_access_iterator<I>) {
    return std::random_access_iterator_tag{};
  } else if constexpr (std::bidirectional_iterator<I>) {
    return std::bidirectional_iterator_tag{};
  } else if constexpr (std::forward_iterator<I>) {
    return std::forward_iterator_tag{};
  } else if constexpr (std::input_iterator<I>) {
    return std::input_iterator_tag{};
  }
}

template <class I>
using exposition_only_iterator_to_tag_t =
    decltype(exposition_only_iterator_to_tag<I>()); // @*exposition only*@

template <typename V, typename ToType>
concept exposition_only_convertible_to_charN_t_view =
    exposition_only_code_unit_to<ToType> && std::ranges::view<V> &&
    std::convertible_to<std::ranges::range_reference_t<V>, ToType>;

template <exposition_only_convertible_to_charN_t_view<char8_t> V>
class as_char8_t_view : public std::ranges::view_interface<as_char8_t_view<V>> {
  V base_ = V(); // @*exposition only*@

  template <bool Const>
  class exposition_only_iterator; // @*exposition only*@
  template <bool Const>
  class exposition_only_sentinel; // @*exposition only*@

public:
  constexpr as_char8_t_view()
    requires std::default_initializable<V>
  = default;
  constexpr as_char8_t_view(V base)
      : base_(std::move(base)) { }

  constexpr V& base() & {
    return base_;
  }
  constexpr const V& base() const&
    requires std::copy_constructible<V>
  {
    return base_;
  }
  constexpr V base() && {
    return std::move(base_);
  }

  constexpr exposition_only_iterator<false> begin() {
    return exposition_only_iterator<false>{std::ranges::begin(base_)};
  }
  constexpr exposition_only_iterator<true> begin() const
    requires std::ranges::range<const V>
  {
    return exposition_only_iterator<true>{std::ranges::begin(base_)};
  }

  constexpr exposition_only_sentinel<false> end() {
    return exposition_only_sentinel<false>{std::ranges::end(base_)};
  }
  constexpr exposition_only_iterator<false> end()
    requires std::ranges::common_range<V>
  {
    return exposition_only_iterator<false>{std::ranges::end(base_)};
  }
  constexpr exposition_only_sentinel<true> end() const
    requires std::ranges::range<const V>
  {
    return exposition_only_sentinel<true>{std::ranges::end(base_)};
  }
  constexpr exposition_only_iterator<true> end() const
    requires std::ranges::common_range<const V>
  {
    return exposition_only_iterator<true>{std::ranges::end(base_)};
  }

  constexpr auto size()
    requires std::ranges::sized_range<V>
  {
    return std::ranges::size(base_);
  }
  constexpr auto size() const
    requires std::ranges::sized_range<const V>
  {
    return std::ranges::size(base_);
  }
};

template <exposition_only_convertible_to_charN_t_view<char8_t> V>
template <bool Const>
class as_char8_t_view<V>::exposition_only_iterator
    : public beman::iterator_interface::iterator_interface<
          exposition_only_iterator_to_tag_t<
              std::ranges::iterator_t<exposition_only_maybe_const<Const, V>>>,
          char8_t, char8_t, void, std::ptrdiff_t> {
public:
  using reference_type = char8_t;

private:
  using exposition_only_iterator_type = std::ranges::iterator_t<
      exposition_only_maybe_const<Const, V>>; // @*exposition only*@

  friend beman::iterator_interface::iterator_interface_access;

  constexpr exposition_only_iterator_type& base_reference() noexcept {
    return it_;
  } // @*exposition only*@
  constexpr exposition_only_iterator_type base_reference() const {
    return it_;
  } // @*exposition only*@

  exposition_only_iterator_type it_ =
      exposition_only_iterator_type(); // @*exposition only*@

public:
  constexpr exposition_only_iterator() = default;
  constexpr exposition_only_iterator(exposition_only_iterator_type it)
      : it_(std::move(it)) { }

  constexpr reference_type operator*() const {
    return *it_;
  }
};

template <exposition_only_convertible_to_charN_t_view<char8_t> V>
template <bool Const>
class as_char8_t_view<V>::exposition_only_sentinel {
  using exposition_only_base =
      exposition_only_maybe_const<Const, V>; // @*exposition only*@
  using exposition_only_sentinel_type =
      std::ranges::sentinel_t<exposition_only_base>; // @*exposition only*@

  exposition_only_sentinel_type end_ =
      exposition_only_sentinel_type(); // @*exposition only*@

public:
  constexpr exposition_only_sentinel() = default;
  constexpr explicit exposition_only_sentinel(exposition_only_sentinel_type end)
      : end_(std::move(end)) { }
  constexpr exposition_only_sentinel(exposition_only_sentinel<!Const> i)
    requires Const &&
      std::convertible_to<std::ranges::sentinel_t<V>,
                          std::ranges::sentinel_t<exposition_only_base>>;

  constexpr exposition_only_sentinel_type base() const {
    return end_;
  }

  template <bool OtherConst>
    requires std::sentinel_for<
        exposition_only_sentinel_type,
        std::ranges::iterator_t<exposition_only_maybe_const<OtherConst, V>>>
  friend constexpr bool operator==(const exposition_only_iterator<OtherConst>& x,
                                   const exposition_only_sentinel& y) {
    return x.it_ == y.end_;
  }

  template <bool OtherConst>
    requires std::sized_sentinel_for<
        exposition_only_sentinel_type,
        std::ranges::iterator_t<exposition_only_maybe_const<OtherConst, V>>>
  friend constexpr std::ranges::range_difference_t<
      exposition_only_maybe_const<OtherConst, V>>
  operator-(const exposition_only_iterator<OtherConst>& x,
            const exposition_only_sentinel& y) {
    return x.it_ - y.end_;
  }

  template <bool OtherConst>
    requires std::sized_sentinel_for<
        exposition_only_sentinel_type,
        std::ranges::iterator_t<exposition_only_maybe_const<OtherConst, V>>>
  friend constexpr std::ranges::range_difference_t<
      exposition_only_maybe_const<OtherConst, V>>
  operator-(const exposition_only_sentinel& y,
            const exposition_only_iterator<OtherConst>& x) {
    return y.end_ - x.it_;
  }
};

template <class R>
as_char8_t_view(R&&) -> as_char8_t_view<std::views::all_t<R>>;

template <exposition_only_convertible_to_charN_t_view<char16_t> V>
class as_char16_t_view : public std::ranges::view_interface<as_char16_t_view<V>> {
  V base_ = V(); // @*exposition only*@

  template <bool Const>
  class exposition_only_iterator; // @*exposition only*@
  template <bool Const>
  class exposition_only_sentinel; // @*exposition only*@

public:
  constexpr as_char16_t_view()
    requires std::default_initializable<V>
  = default;
  constexpr as_char16_t_view(V base)
      : base_(std::move(base)) { }

  constexpr V& base() & {
    return base_;
  }
  constexpr const V& base() const&
    requires std::copy_constructible<V>
  {
    return base_;
  }
  constexpr V base() && {
    return std::move(base_);
  }

  constexpr exposition_only_iterator<false> begin() {
    return exposition_only_iterator<false>{std::ranges::begin(base_)};
  }
  constexpr exposition_only_iterator<true> begin() const
    requires std::ranges::range<const V>
  {
    return exposition_only_iterator<true>{std::ranges::begin(base_)};
  }

  constexpr exposition_only_sentinel<false> end() {
    return exposition_only_sentinel<false>{std::ranges::end(base_)};
  }
  constexpr exposition_only_iterator<false> end()
    requires std::ranges::common_range<V>
  {
    return exposition_only_iterator<false>{std::ranges::end(base_)};
  }
  constexpr exposition_only_sentinel<true> end() const
    requires std::ranges::range<const V>
  {
    return exposition_only_sentinel<true>{std::ranges::end(base_)};
  }
  constexpr exposition_only_iterator<true> end() const
    requires std::ranges::common_range<const V>
  {
    return exposition_only_iterator<true>{std::ranges::end(base_)};
  }

  constexpr auto size()
    requires std::ranges::sized_range<V>
  {
    return std::ranges::size(base_);
  }
  constexpr auto size() const
    requires std::ranges::sized_range<const V>
  {
    return std::ranges::size(base_);
  }
};

template <exposition_only_convertible_to_charN_t_view<char16_t> V>
template <bool Const>
class as_char16_t_view<V>::exposition_only_iterator
    : public beman::iterator_interface::iterator_interface<
          exposition_only_iterator_to_tag_t<
              std::ranges::iterator_t<exposition_only_maybe_const<Const, V>>>,
          char16_t, char16_t, void, std::ptrdiff_t> {
public:
  using reference_type = char16_t;

private:
  using exposition_only_iterator_type = std::ranges::iterator_t<
      exposition_only_maybe_const<Const, V>>; // @*exposition only*@

  friend beman::iterator_interface::iterator_interface_access;

  constexpr exposition_only_iterator_type& base_reference() noexcept {
    return it_;
  } // @*exposition only*@
  constexpr exposition_only_iterator_type base_reference() const {
    return it_;
  } // @*exposition only*@

  exposition_only_iterator_type it_ =
      exposition_only_iterator_type(); // @*exposition only*@

public:
  constexpr exposition_only_iterator() = default;
  constexpr exposition_only_iterator(exposition_only_iterator_type it)
      : it_(std::move(it)) { }

  constexpr reference_type operator*() const {
    return *it_;
  }
};

template <exposition_only_convertible_to_charN_t_view<char16_t> V>
template <bool Const>
class as_char16_t_view<V>::exposition_only_sentinel {
  using exposition_only_base =
      exposition_only_maybe_const<Const, V>; // @*exposition only*@
  using exposition_only_sentinel_type =
      std::ranges::sentinel_t<exposition_only_base>; // @*exposition only*@

  exposition_only_sentinel_type end_ =
      exposition_only_sentinel_type(); // @*exposition only*@

public:
  constexpr exposition_only_sentinel() = default;
  constexpr explicit exposition_only_sentinel(exposition_only_sentinel_type end)
      : end_(std::move(end)) { }
  constexpr exposition_only_sentinel(exposition_only_sentinel<!Const> i)
    requires Const &&
      std::convertible_to<std::ranges::sentinel_t<V>,
                          std::ranges::sentinel_t<exposition_only_base>>;

  constexpr exposition_only_sentinel_type base() const {
    return end_;
  }

  template <bool OtherConst>
    requires std::sentinel_for<
        exposition_only_sentinel_type,
        std::ranges::iterator_t<exposition_only_maybe_const<OtherConst, V>>>
  friend constexpr bool operator==(const exposition_only_iterator<OtherConst>& x,
                                   const exposition_only_sentinel& y) {
    return x.it_ == y.end_;
  }

  template <bool OtherConst>
    requires std::sized_sentinel_for<
        exposition_only_sentinel_type,
        std::ranges::iterator_t<exposition_only_maybe_const<OtherConst, V>>>
  friend constexpr std::ranges::range_difference_t<
      exposition_only_maybe_const<OtherConst, V>>
  operator-(const exposition_only_iterator<OtherConst>& x,
            const exposition_only_sentinel& y) {
    return x.it_ - y.end_;
  }

  template <bool OtherConst>
    requires std::sized_sentinel_for<
        exposition_only_sentinel_type,
        std::ranges::iterator_t<exposition_only_maybe_const<OtherConst, V>>>
  friend constexpr std::ranges::range_difference_t<
      exposition_only_maybe_const<OtherConst, V>>
  operator-(const exposition_only_sentinel& y,
            const exposition_only_iterator<OtherConst>& x) {
    return y.end_ - x.it_;
  }
};

template <class R>
as_char16_t_view(R&&) -> as_char16_t_view<std::views::all_t<R>>;

template <exposition_only_convertible_to_charN_t_view<char32_t> V>
class as_char32_t_view : public std::ranges::view_interface<as_char32_t_view<V>> {
  V base_ = V(); // @*exposition only*@

  template <bool Const>
  class exposition_only_iterator; // @*exposition only*@
  template <bool Const>
  class exposition_only_sentinel; // @*exposition only*@

public:
  constexpr as_char32_t_view()
    requires std::default_initializable<V>
  = default;
  constexpr as_char32_t_view(V base)
      : base_(std::move(base)) { }

  constexpr V& base() & {
    return base_;
  }
  constexpr const V& base() const&
    requires std::copy_constructible<V>
  {
    return base_;
  }
  constexpr V base() && {
    return std::move(base_);
  }

  constexpr exposition_only_iterator<false> begin() {
    return exposition_only_iterator<false>{std::ranges::begin(base_)};
  }
  constexpr exposition_only_iterator<true> begin() const
    requires std::ranges::range<const V>
  {
    return exposition_only_iterator<true>{std::ranges::begin(base_)};
  }

  constexpr exposition_only_sentinel<false> end() {
    return exposition_only_sentinel<false>{std::ranges::end(base_)};
  }
  constexpr exposition_only_iterator<false> end()
    requires std::ranges::common_range<V>
  {
    return exposition_only_iterator<false>{std::ranges::end(base_)};
  }
  constexpr exposition_only_sentinel<true> end() const
    requires std::ranges::range<const V>
  {
    return exposition_only_sentinel<true>{std::ranges::end(base_)};
  }
  constexpr exposition_only_iterator<true> end() const
    requires std::ranges::common_range<const V>
  {
    return exposition_only_iterator<true>{std::ranges::end(base_)};
  }

  constexpr auto size()
    requires std::ranges::sized_range<V>
  {
    return std::ranges::size(base_);
  }
  constexpr auto size() const
    requires std::ranges::sized_range<const V>
  {
    return std::ranges::size(base_);
  }
};

template <exposition_only_convertible_to_charN_t_view<char32_t> V>
template <bool Const>
class as_char32_t_view<V>::exposition_only_iterator
    : public beman::iterator_interface::iterator_interface<
          exposition_only_iterator_to_tag_t<
              std::ranges::iterator_t<exposition_only_maybe_const<Const, V>>>,
          char32_t, char32_t, void, std::ptrdiff_t> {
public:
  using reference_type = char32_t;

private:
  using exposition_only_iterator_type = std::ranges::iterator_t<
      exposition_only_maybe_const<Const, V>>; // @*exposition only*@

  friend beman::iterator_interface::iterator_interface_access;

  constexpr exposition_only_iterator_type& base_reference() noexcept {
    return it_;
  } // @*exposition only*@
  constexpr exposition_only_iterator_type base_reference() const {
    return it_;
  } // @*exposition only*@

  exposition_only_iterator_type it_ =
      exposition_only_iterator_type(); // @*exposition only*@

public:
  constexpr exposition_only_iterator() = default;
  constexpr exposition_only_iterator(exposition_only_iterator_type it)
      : it_(std::move(it)) { }

  constexpr reference_type operator*() const {
    return *it_;
  }
};

template <exposition_only_convertible_to_charN_t_view<char32_t> V>
template <bool Const>
class as_char32_t_view<V>::exposition_only_sentinel {
  using exposition_only_base =
      exposition_only_maybe_const<Const, V>; // @*exposition only*@
  using exposition_only_sentinel_type =
      std::ranges::sentinel_t<exposition_only_base>; // @*exposition only*@

  exposition_only_sentinel_type end_ =
      exposition_only_sentinel_type(); // @*exposition only*@

public:
  constexpr exposition_only_sentinel() = default;
  constexpr explicit exposition_only_sentinel(exposition_only_sentinel_type end)
      : end_(std::move(end)) { }
  constexpr exposition_only_sentinel(exposition_only_sentinel<!Const> i)
    requires Const &&
      std::convertible_to<std::ranges::sentinel_t<V>,
                          std::ranges::sentinel_t<exposition_only_base>>;

  constexpr exposition_only_sentinel_type base() const {
    return end_;
  }

  template <bool OtherConst>
    requires std::sentinel_for<
        exposition_only_sentinel_type,
        std::ranges::iterator_t<exposition_only_maybe_const<OtherConst, V>>>
  friend constexpr bool operator==(const exposition_only_iterator<OtherConst>& x,
                                   const exposition_only_sentinel& y) {
    return x.it_ == y.end_;
  }

  template <bool OtherConst>
    requires std::sized_sentinel_for<
        exposition_only_sentinel_type,
        std::ranges::iterator_t<exposition_only_maybe_const<OtherConst, V>>>
  friend constexpr std::ranges::range_difference_t<
      exposition_only_maybe_const<OtherConst, V>>
  operator-(const exposition_only_iterator<OtherConst>& x,
            const exposition_only_sentinel& y) {
    return x.it_ - y.end_;
  }

  template <bool OtherConst>
    requires std::sized_sentinel_for<
        exposition_only_sentinel_type,
        std::ranges::iterator_t<exposition_only_maybe_const<OtherConst, V>>>
  friend constexpr std::ranges::range_difference_t<
      exposition_only_maybe_const<OtherConst, V>>
  operator-(const exposition_only_sentinel& y,
            const exposition_only_iterator<OtherConst>& x) {
    return y.end_ - x.it_;
  }
};

template <class R>
as_char32_t_view(R&&) -> as_char32_t_view<std::views::all_t<R>>;

/* !PAPER */

namespace detail {

  template <exposition_only_code_unit_to ToType, std::ranges::view V>
  using as_charN_t_view =
      std::conditional_t<std::is_same_v<ToType, char8_t>, as_char8_t_view<V>,
                         std::conditional_t<std::is_same_v<ToType, char16_t>,
                                            as_char16_t_view<V>, as_char32_t_view<V>>>;

  template <exposition_only_code_unit_to ToType>
  struct as_code_unit_impl
      : std::ranges::range_adaptor_closure<as_code_unit_impl<ToType>> {
    template <std::ranges::range R>
      requires std::convertible_to<std::ranges::range_reference_t<R>, ToType>
    constexpr auto operator()(R&& r) const {
      using T = std::remove_cvref_t<R>;
      if constexpr (exposition_only_is_empty_view<T>) {
        return std::ranges::empty_view<ToType>{};
      } else if constexpr (std::is_bounded_array_v<T>) {
        constexpr auto n = std::extent_v<T>;
        auto first{std::ranges::begin(r)};
        auto last{std::ranges::end(r)};
        if (n && !r[n - 1]) {
          --last;
        }
        std::ranges::subrange subrange(first, last);
        return as_charN_t_view<ToType, decltype(subrange)>(std::move(subrange));
      } else {
        auto view = std::views::all(std::forward<R>(r));
        return as_charN_t_view<ToType, decltype(view)>(std::move(view));
      }
    }
  };

} // namespace detail

inline constexpr detail::as_code_unit_impl<char8_t> as_char8_t;

inline constexpr detail::as_code_unit_impl<char16_t> as_char16_t;

inline constexpr detail::as_code_unit_impl<char32_t> as_char32_t;

/* PAPER:   inline constexpr @*unspecified*@ as_char8_t;  */
/* PAPER:                                                 */
/* PAPER:   inline constexpr @*unspecified*@ as_char16_t; */
/* PAPER:                                                 */
/* PAPER:   inline constexpr @*unspecified*@ as_char32_t; */
/* PAPER:                                                 */
/* PAPER: }                                               */

} // namespace beman::utf_view

template <class V>
inline constexpr bool std::ranges::enable_borrowed_range<beman::utf_view::as_char8_t_view<V>> =
    std::ranges::enable_borrowed_range<V>;

template <class V>
inline constexpr bool std::ranges::enable_borrowed_range<beman::utf_view::as_char16_t_view<V>> =
    std::ranges::enable_borrowed_range<V>;

template <class V>
inline constexpr bool std::ranges::enable_borrowed_range<beman::utf_view::as_char32_t_view<V>> =
    std::ranges::enable_borrowed_range<V>;

/* PAPER: namespace std::ranges {                                                                                 */
/* PAPER:                                                                                                         */
/* PAPER:   template <class V>                                                                                    */
/* PAPER:   inline constexpr bool enable_borrowed_range<std::uc::as_char8_t_view<V>> = enable_borrowed_range<V>;  */
/* PAPER:                                                                                                         */
/* PAPER:   template <class V>                                                                                    */
/* PAPER:   inline constexpr bool enable_borrowed_range<std::uc::as_char16_t_view<V>> = enable_borrowed_range<V>; */
/* PAPER:                                                                                                         */
/* PAPER:   template <class V>                                                                                    */
/* PAPER:   inline constexpr bool enable_borrowed_range<std::uc::as_char32_t_view<V>> = enable_borrowed_range<V>; */
/* PAPER:                                                                                                         */
/* PAPER: }                                                                                                       */

#endif // BEMAN_UTF_VIEW_CODE_UNIT_VIEW_HPP
