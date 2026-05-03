# SPDX-License-Identifier: BSL-1.0

cmake_minimum_required(VERSION 3.30...4.3)

include(infra/cmake/enable-experimental-import-std.cmake)

project(
    beman.utf_view # CMake Project Name, which is also the name of the top-level
    # targets (e.g., library, executable, etc.).
    DESCRIPTION "C++29 UTF Transcoding Views"
    LANGUAGES CXX
    VERSION 0.1.0
)

# [CMAKE.SKIP_TESTS]
option(
    BEMAN_UTF_VIEW_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# [CMAKE.SKIP_EXAMPLES]
option(
    BEMAN_UTF_VIEW_BUILD_EXAMPLES
    "Enable building examples. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

option(
    BEMAN_UTF_VIEW_USE_MODULES
    "Provide beman.transform_view as a C++ module"
    OFF
)

if(BEMAN_UTF_VIEW_USE_MODULES)
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
endif()

configure_file(
    "${PROJECT_SOURCE_DIR}/include/beman/utf_view/config_generated.hpp.in"
    "${PROJECT_BINARY_DIR}/include/beman/utf_view/config_generated.hpp"
    @ONLY
)

option(
    BEMAN_UTF_VIEW_BUILD_PAPER
    "Enable building paper. Default: OFF. Values: { ON, OFF }."
    OFF
)

# for find of beman_install_library and configure_build_telemetry
include(infra/cmake/beman-install-library.cmake)
include(infra/cmake/BuildTelemetryConfig.cmake)

if(BEMAN_UTF_VIEW_USE_MODULES)
    set(BEMAN_TRANSFORM_VIEW_USE_MODULES ON)
endif()
find_package(beman.transform_view REQUIRED)

if(BEMAN_UTF_VIEW_USE_MODULES)
    add_library(beman.utf_view STATIC)
else()
    add_library(beman.utf_view INTERFACE)
endif()
add_library(beman::utf_view ALIAS beman.utf_view)

if(BEMAN_UTF_VIEW_USE_MODULES)
    target_sources(
        beman.utf_view
        PUBLIC
            FILE_SET CXX_MODULES
            FILE_SET HEADERS
                BASE_DIRS
                    "${CMAKE_CURRENT_SOURCE_DIR}/include"
                    "${CMAKE_CURRENT_BINARY_DIR}/include"
    )
    set_target_properties(beman.utf_view PROPERTIES CXX_MODULE_STD ON)
    target_compile_features(beman.utf_view PUBLIC cxx_std_23)
else()
    target_sources(
        beman.utf_view
        PUBLIC
            FILE_SET HEADERS
                BASE_DIRS
                    "${CMAKE_CURRENT_SOURCE_DIR}/include"
                    "${CMAKE_CURRENT_BINARY_DIR}/include"
    )
    set_target_properties(
        beman.utf_view
        PROPERTIES VERIFY_INTERFACE_HEADER_SETS ${PROJECT_IS_TOP_LEVEL}
    )
endif()

if(BEMAN_UTF_VIEW_USE_MODULES)
    target_link_libraries(beman.utf_view PRIVATE beman::transform_view)
else()
    target_link_libraries(beman.utf_view INTERFACE beman::transform_view)
endif()

add_subdirectory(include/beman/utf_view)

if(BEMAN_USE_FETCH_CONTENT_ENABLED)
    beman_install_library(
        beman.utf_view
        TARGETS beman.utf_view beman.transform_view
    )
else()
    beman_install_library(
        beman.utf_view
        TARGETS beman.utf_view
        DEPENDENCIES "[===[beman.transform_view]===]"
    )
endif()
configure_build_telemetry()

if(BEMAN_UTF_VIEW_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/beman/utf_view)
endif()

if(BEMAN_UTF_VIEW_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BEMAN_UTF_VIEW_BUILD_PAPER)
    add_subdirectory(papers)
endif()
