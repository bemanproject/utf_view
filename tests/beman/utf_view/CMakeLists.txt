# SPDX-License-Identifier: BSL-1.0

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(
    beman_utf_view_test_lib
    STATIC
    code_unit_view.test.cpp
    detail/concepts.test.cpp
    endian_view.test.cpp
    framework.cpp
    null_term.test.cpp
    std_archetypes/exposition_only.test.cpp
    std_archetypes/iterator.test.cpp
    to_utf_view.test.cpp
)

target_link_libraries(beman_utf_view_test_lib beman::utf_view)

add_executable(beman_utf_view_test main.test.cpp)

target_link_libraries(
    beman_utf_view_test
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,beman_utf_view_test_lib>"
)

add_test(NAME beman_utf_view_test COMMAND beman_utf_view_test)

if(BEMAN_UTF_VIEW_USE_MODULES)
    set_target_properties(beman_utf_view_test_lib PROPERTIES CXX_MODULE_STD ON)
    set_target_properties(beman_utf_view_test PROPERTIES CXX_MODULE_STD ON)
endif()
