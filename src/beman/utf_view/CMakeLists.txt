# SPDX-License-Identifier: BSL-1.0

#   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

add_library(beman_utf_view INTERFACE)

include(GNUInstallDirs)

add_library(beman.utf_view)
add_library(beman::utf_view ALIAS beman.utf_view)

target_sources(beman.utf_view PRIVATE utf_view.cpp)

target_sources(
    beman.utf_view
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../../../include
            FILES
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/beman/utf_view/code_unit_view.hpp
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/beman/utf_view/detail/concepts.hpp
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/beman/utf_view/detail/constexpr_unless_msvc.hpp
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/beman/utf_view/detail/fake_inplace_vector.hpp
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/beman/utf_view/detail/nontype_t_polyfill.hpp
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/beman/utf_view/endian_view.hpp
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/beman/utf_view/null_term.hpp
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/beman/utf_view/to_utf_view.hpp
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/beman/utf_view/utf_view.hpp
)

set_target_properties(
    beman.utf_view
    PROPERTIES VERIFY_INTERFACE_HEADER_SETS ON EXPORT_NAME utf_view
)

target_link_libraries(beman.utf_view PUBLIC beman::transform_view)

install(
    TARGETS beman.utf_view beman.transform_view
    COMPONENT beman.utf_view
    EXPORT beman.utf_view
    DESTINATION ${CMAKE_INSTALL_LIBDIR}$<$<CONFIG:Debug>:/debug>
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}$<$<CONFIG:Debug>:/debug>
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(BEMAN_UTF_VIEW_INSTALL_CONFIG_FILE_PACKAGE)
    include(CMakePackageConfigHelpers)

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/beman.utf_view-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/beman.utf_view-config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beman.utf_view"
        PATH_VARS PROJECT_NAME PROJECT_VERSION
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/beman.utf_view-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY ExactVersion
    )

    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/beman.utf_view-config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/beman.utf_view-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beman.utf_view"
        COMPONENT beman.utf_view
    )

    install(
        EXPORT beman.utf_view
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beman.utf_view"
        NAMESPACE beman::
        FILE beman.utf_view-targets.cmake
        COMPONENT beman.utf_view
    )
endif()
