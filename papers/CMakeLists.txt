# SPDX-License-Identifier: BSL-1.0

#   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(FetchContent)

FetchContent_Declare(
  wg21
  GIT_REPOSITORY https://github.com/mpark/wg21.git
  GIT_TAG 49c655869c14d80645962b6fceb503249f69bc9e
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(wg21)

add_custom_command(
  OUTPUT P2728.html
  COMMAND SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} OUTDIR=${CMAKE_CURRENT_BINARY_DIR} make -f ${wg21_SOURCE_DIR}/Makefile html
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/P2728.md
  VERBATIM)
add_custom_target(p2728 ALL DEPENDS P2728.html)

add_custom_command(
  OUTPUT P3705.html
  COMMAND SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} OUTDIR=${CMAKE_CURRENT_BINARY_DIR} make -f ${wg21_SOURCE_DIR}/Makefile html
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/P3705.md
  VERBATIM)
add_custom_target(p3705 ALL DEPENDS P3705.html p2728)

add_custom_command(
  OUTPUT P4030.html
  COMMAND SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} OUTDIR=${CMAKE_CURRENT_BINARY_DIR} make -f ${wg21_SOURCE_DIR}/Makefile html
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/P4030.md
  VERBATIM)
add_custom_target(p4030 ALL DEPENDS P4030.html p3705)
