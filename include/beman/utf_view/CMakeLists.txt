# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

target_sources(
    beman.utf_view
    PUBLIC
        FILE_SET HEADERS
            FILES
                code_unit_view.hpp
                detail/concepts.hpp
                detail/constant_wrapper_polyfill.hpp
                detail/constexpr_unless_msvc.hpp
                detail/fake_inplace_vector.hpp
                endian_view.hpp
                null_term.hpp
                to_utf_view.hpp
                utf_view.hpp
)
