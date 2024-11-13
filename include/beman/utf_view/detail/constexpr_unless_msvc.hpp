// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef BEMAN_UTF_VIEW_DETAIL_CONSTEXPR_UNLESS_MSVC_HPP
#define BEMAN_UTF_VIEW_DETAIL_CONSTEXPR_UNLESS_MSVC_HPP

// msvc constexpr bugs can be induced by a light breeze
// https://developercommunity.visualstudio.com/t/std::initializer_list-data-member-of-cla/10622889
// https://developercommunity.visualstudio.com/t/MSVC-complains-about-uninvoked-implicitl/10585513
// https://developercommunity.visualstudio.com/t/Constraint-not-applied-to-defaulted-cons/10715859
#ifdef _MSC_VER
#define CONSTEXPR_UNLESS_MSVC
#else
#define CONSTEXPR_UNLESS_MSVC constexpr
#endif

#endif // BEMAN_UTF_VIEW_DETAIL_CONSTEXPR_UNLESS_MSVC_HPP
