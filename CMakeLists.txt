# SPDX-License-Identifier: BSL-1.0

#   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.28...4.2)
project(
    beman.utf_view
    DESCRIPTION "C++29 UTF Transcoding Views"
    LANGUAGES CXX
    VERSION 0.0.1
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

# for find of beman-install-library
include(infra/cmake/beman-install-library-config.cmake)

add_library(beman.utf_view INTERFACE)
add_library(beman::utf_view ALIAS beman.utf_view)

target_sources(
    beman.utf_view
    PUBLIC FILE_SET HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(beman.utf_view PROPERTIES VERIFY_INTERFACE_HEADER_SETS ON)

add_subdirectory(include/beman/utf_view)

include(CTest)

find_package(beman-install-library REQUIRED)
beman_install_library(beman.utf_view)

find_package(beman::transform_view REQUIRED)

target_link_libraries(beman.utf_view INTERFACE beman::transform_view)

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
