module;

#include <beman/utf_view/detail/constexpr_unless_msvc.hpp>
#include <cassert>

export module beman.utf_view;

import beman.transform_view;
import std;

#define BEMAN_UTF_VIEW_INCLUDED_FROM_INTERFACE_UNIT
export {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <beman/utf_view/detail/concepts.hpp>
#include <beman/utf_view/detail/constant_wrapper_polyfill.hpp>
#include <beman/utf_view/detail/fake_inplace_vector.hpp>
#include <beman/utf_view/utf_view.hpp>
#pragma clang diagnostic pop
}
