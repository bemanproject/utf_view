module;

#include <cassert>

export module beman.utf_view;

import beman.transform_view;
import std;

extern "C++" {

#include <beman/utf_view/detail/concepts.hpp>
#include <beman/utf_view/detail/constexpr_unless_msvc.hpp>
#include <beman/utf_view/detail/fake_inplace_vector.hpp>
#include <beman/utf_view/detail/nontype_t_polyfill.hpp>

export {
    #include <beman/utf_view/utf_view.hpp>
}

}
