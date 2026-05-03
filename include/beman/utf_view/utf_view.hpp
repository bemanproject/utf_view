#ifndef BEMAN_UTF_VIEW_UTF_VIEW_HPP
#define BEMAN_UTF_VIEW_UTF_VIEW_HPP

#include <beman/utf_view/config.hpp>

#if BEMAN_UTF_VIEW_USE_MODULES() && \
    !defined(BEMAN_UTF_VIEW_INCLUDED_FROM_INTERFACE_UNIT)

import beman.utf_view;

#else

#include <beman/utf_view/code_unit_view.hpp>
#include <beman/utf_view/endian_view.hpp>
#include <beman/utf_view/null_term.hpp>
#include <beman/utf_view/to_utf_view.hpp>

#endif // BEMAN_UTF_VIEW_USE_MODULES() &&
       // !defined(BEMAN_UTF_VIEW_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_UTF_VIEW_UTF_VIEW_HPP
