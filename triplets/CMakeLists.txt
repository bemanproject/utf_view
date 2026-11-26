# SPDX-License-Identifier: BSL-1.0

cmake_minimum_required(VERSION 4.5)

project(utf_view_smoke LANGUAGES CXX)

enable_testing()

find_package(beman.utf_view 0.3.0 EXACT CONFIG REQUIRED)
message(STATUS "Using beman.utf_view config: ${beman.utf_view_CONFIG}")

add_executable(utf_view_smoke main.cpp)
set_target_properties(
    utf_view_smoke
    PROPERTIES CXX_STANDARD 26 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
)
target_link_libraries(utf_view_smoke PRIVATE beman::utf_view)

add_test(NAME utf_view_smoke COMMAND utf_view_smoke)
