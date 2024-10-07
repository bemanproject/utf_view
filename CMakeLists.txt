# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.27)
project(utfview CXX)

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

add_subdirectory(src/UtfView)

add_subdirectory(paper)
