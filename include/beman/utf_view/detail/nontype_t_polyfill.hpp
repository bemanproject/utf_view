// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef BEMAN_UTF_VIEW_NONTYPE_T_POLYFILL_HPP
#define BEMAN_UTF_VIEW_NONTYPE_T_POLYFILL_HPP

#include <utility>
#include <version>

namespace beman::utf_view::detail {

#ifdef __cpp_lib_function_ref

template <auto V>
using nontype_t = std::nontype_t<V>;

#else

template<auto V>
struct nontype_t {
  explicit nontype_t() = default;
};

#endif

template<auto V> constexpr nontype_t<V> nontype{};

} // namespace beman::utf_view::detail

#endif // BEMAN_UTF_VIEW_NONTYPE_T_POLYFILL_HPP
