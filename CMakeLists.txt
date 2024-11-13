# SPDX-License-Identifier: BSL-1.0

#   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.27)
project(utf_view CXX)

option(
    UTF_VIEW_BUILD_PAPER
    "Enable building paper. Default: OFF. Values: { ON, OFF }."
    OFF
)

# Don't build tests if configuring utf_view as a submodule of another
# CMake project, unless they explicitly set UTF_VIEW_BUILD_TESTS=TRUE,
# or they enabled CTest's BUILD_TESTING option
if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) OR BUILD_TESTING)
    set(UTF_VIEW_BUILD_TESTS_DEFAULT ON)
else()
    set(UTF_VIEW_BUILD_TESTS_DEFAULT OFF)
endif()
option(
  UTF_VIEW_BUILD_TESTS
  "Enable building tests and test infrastructure. Default: ON. Values: { ON, OFF }."
  ${UTF_VIEW_BUILD_TESTS_DEFAULT})

# UTF_VIEW_BUILD_TESTS is used solely to configure CTest's BUILD_TESTING option,
# which is CMake's preferred option for enabling testing when using CTest.
set(BUILD_TESTING ${UTF_VIEW_BUILD_TESTS})

if (BUILD_TESTING)
  include(CTest)
endif()

if (NOT TARGET boost_config)
  add_subdirectory(deps/config)
endif()

if (NOT TARGET boost_assert)
  add_subdirectory(deps/assert)
endif()

if (NOT TARGET boost_stl_interfaces)
  add_definitions(-DBOOST_STL_INTERFACES_ENABLE_DEDUCED_THIS)
  add_subdirectory(deps/stl_interfaces)
else()
  message(FATAL_ERROR "Conflicting dependency: boost_stl_interfaces already exists, but we need our own fork")
endif()

add_subdirectory(src/utf_view)

if (UTF_VIEW_BUILD_PAPER)
  add_subdirectory(paper)
endif()
