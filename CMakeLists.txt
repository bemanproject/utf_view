# SPDX-License-Identifier: BSL-1.0

cmake_minimum_required(VERSION 3.30...4.3)

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
    BEMAN_UTF_VIEW_BUILD_PAPER
    "Enable building paper. Default: OFF. Values: { ON, OFF }."
    OFF
)

# for find of beman_install_library and configure_build_telemetry
include(infra/cmake/beman-install-library.cmake)
include(infra/cmake/BuildTelemetryConfig.cmake)

find_package(beman.transform_view REQUIRED)

add_library(beman.utf_view INTERFACE)
add_library(beman::utf_view ALIAS beman.utf_view)

target_sources(
    beman.utf_view
    PUBLIC FILE_SET HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(
    beman.utf_view
    PROPERTIES VERIFY_INTERFACE_HEADER_SETS ${PROJECT_IS_TOP_LEVEL}
)

target_link_libraries(beman.utf_view INTERFACE beman::transform_view)

add_subdirectory(include/beman/utf_view)

# TODO: Replace with beman_install_library once install with FetchContent deps is sorted out
install(TARGETS beman.utf_view FILE_SET HEADERS)
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
