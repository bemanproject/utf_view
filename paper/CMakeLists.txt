# SPDX-License-Identifier: BSL-1.0

#   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

add_custom_command(
  OUTPUT P2728.html
  COMMAND SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} OUTDIR=${CMAKE_CURRENT_BINARY_DIR} make -C ${CMAKE_CURRENT_SOURCE_DIR}/../deps/wg21 html
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/P2728.md
  VERBATIM)
add_custom_target(p2728 DEPENDS P2728.html)
