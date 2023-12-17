// SPDX-License-Identifier: GPL-3.0-only

#ifndef UTFVIEW_DETAIL_CONSTEXPR_UNLESS_MSVC_HPP
#define UTFVIEW_DETAIL_CONSTEXPR_UNLESS_MSVC_HPP

// msvc constexpr bugs can be induced by a light breeze
// https://developercommunity.visualstudio.com/t/std::initializer_list-data-member-of-cla/10622889
// https://developercommunity.visualstudio.com/t/MSVC-complains-about-uninvoked-implicitl/10585513
// https://developercommunity.visualstudio.com/t/Constraint-not-applied-to-defaulted-cons/10715859
#ifdef _MSC_VER
#define CONSTEXPR_UNLESS_MSVC
#else
#define CONSTEXPR_UNLESS_MSVC constexpr
#endif

#endif // UTFVIEW_DETAIL_CONSTEXPR_UNLESS_MSVC_HPP
