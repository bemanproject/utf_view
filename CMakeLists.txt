# SPDX-License-Identifier: BSL-1.0

#   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.27)
project(beman.utf_view CXX)

include(FetchContent)

option(
    UTF_VIEW_BUILD_TESTING
    "Enable building tests and test infrastructure. Default: ON. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

option(
    BEMAN_UTF_VIEW_BUILD_PAPER
    "Enable building paper. Default: OFF. Values: { ON, OFF }."
    OFF
)

FetchContent_Declare(
    beman.iterator_interface
    GIT_REPOSITORY https://github.com/beman-project/iterator_interface.git
    GIT_TAG 94adc3ea613384dc9da64c9de1a7d9a21395ede7
    EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(beman.iterator_interface)

add_subdirectory(src/beman/utf_view)

if (BEMAN_UTF_VIEW_BUILD_PAPER)
  add_subdirectory(paper)
endif()

if (UTF_VIEW_BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests/beman/utf_view)
endif()
