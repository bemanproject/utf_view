# SPDX-License-Identifier: BSL-1.0

#   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

add_library(
  utf_view_test_lib
  STATIC
  code_unit_view.t.cpp
  detail/concepts.t.cpp
  framework.cpp
  null_term.t.cpp
  readme_examples.t.cpp
  std_archetypes/exposition_only.t.cpp
  std_archetypes/iterator.t.cpp
  to_utf_view.t.cpp)

target_link_libraries(utf_view_test_lib utf_view)

add_executable(
  utf_view_test
  main.t.cpp)

target_link_libraries(utf_view_test "$<LINK_LIBRARY:WHOLE_ARCHIVE,utf_view_test_lib>")

add_test(NAME utf_view_test COMMAND utf_view_test)
