# SPDX-License-Identifier: BSL-1.0

#   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(
    beman_utf_view_test_lib
    STATIC
    code_unit_view.t.cpp
    detail/concepts.t.cpp
    framework.cpp
    null_term.t.cpp
    std_archetypes/exposition_only.t.cpp
    std_archetypes/iterator.t.cpp
    to_utf_view.t.cpp
)

target_link_libraries(beman_utf_view_test_lib beman::utf_view)

add_executable(beman_utf_view_test main.t.cpp)

target_link_libraries(
    beman_utf_view_test
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,beman_utf_view_test_lib>"
)

add_test(NAME beman_utf_view_test COMMAND beman_utf_view_test)
