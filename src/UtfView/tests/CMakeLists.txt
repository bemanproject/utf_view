# SPDX-License-Identifier: BSL-1.0

#   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

add_library(
  utfview_test_lib
  STATIC
  code_unit_view.t.cpp
  detail/concepts.t.cpp
  framework.cpp
  null_term.t.cpp
  readme_examples.t.cpp
  std_archetypes/exposition_only.t.cpp
  std_archetypes/iterator.t.cpp
  to_utf_view.t.cpp)

target_link_libraries(utfview_test_lib utfview)

add_executable(
  utfview_test
  main.t.cpp)

target_link_libraries(utfview_test "$<LINK_LIBRARY:WHOLE_ARCHIVE,utfview_test_lib>")

add_test(NAME utfview_test COMMAND utfview_test)
