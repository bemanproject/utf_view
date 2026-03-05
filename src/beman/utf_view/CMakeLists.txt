# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

add_library(beman.utf_view.module STATIC)
add_library(beman::utf_view.module ALIAS beman.utf_view.module)

target_sources(
    beman.utf_view.module
    PUBLIC FILE_SET CXX_MODULES FILES utf_view.cppm
)

target_include_directories(
    beman.utf_view.module
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_definitions(
  beman.utf_view.module
  PUBLIC BEMAN_HAS_MODULES)

set_target_properties(
    beman.utf_view.module
    PROPERTIES CXX_MODULE_STD ON
)

target_link_libraries(beman.utf_view.module beman::transform_view.module)
