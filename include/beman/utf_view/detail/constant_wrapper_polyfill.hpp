// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef BEMAN_UTF_VIEW_CONSTANT_WRAPPER_POLYFILL_HPP
#define BEMAN_UTF_VIEW_CONSTANT_WRAPPER_POLYFILL_HPP

#include <utility>
#include <version>

namespace beman::utf_view::detail {

#ifdef __cpp_lib_constant_wrapper

template <auto X, typename T>
using constant_wrapper = std::constant_wrapper<X, T>;

template<auto X> constexpr auto cw{std::cw<X>};

#else

template<auto X, typename T>
struct constant_wrapper {
  static constexpr const auto value = X;
};

template<auto X> constexpr auto cw{constant_wrapper<X, decltype(X)>{}};

#endif


} // namespace beman::utf_view::detail

#endif // BEMAN_UTF_VIEW_CONSTANT_WRAPPER_POLYFILL_HPP
