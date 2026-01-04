# SPDX-License-Identifier: BSL-1.0

target_sources(
    beman.utf_view
    PUBLIC
        FILE_SET HEADERS
            FILES
                code_unit_view.hpp
                detail/concepts.hpp
                detail/constexpr_unless_msvc.hpp
                detail/fake_inplace_vector.hpp
                detail/nontype_t_polyfill.hpp
                null_term.hpp
                to_utf_view.hpp
                utf_view.hpp
)
