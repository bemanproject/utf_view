# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 4.5)

project(utf_view_examples LANGUAGES CXX)

if(${PROJECT_IS_TOP_LEVEL})
    find_package(beman.utf_view 0.3.0 EXACT CONFIG REQUIRED)
    message(STATUS "Using beman.utf_view config: ${beman.utf_view_CONFIG}")

    if(CMAKE_CXX_STDLIB_MODULES_JSON)
        set(BEMAN_UTF_VIEW_USE_MODULES ON)
        set(BEMAN_TRANSFORM_VIEW_USE_MODULES ON)
    endif()

    enable_testing()
endif()

set(ALL_EXAMPLES readme_examples null_term_argv_environ)

message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(beman.utf_view.examples.${example})
    target_sources(beman.utf_view.examples.${example} PRIVATE ${example}.cpp)
    target_link_libraries(
        beman.utf_view.examples.${example}
        PRIVATE beman::utf_view
    )
    add_test(
        NAME beman_utf_view_example_${example}
        COMMAND beman.utf_view.examples.${example}
    )
    if(BEMAN_UTF_VIEW_USE_MODULES)
        set_target_properties(
            beman.utf_view.examples.${example}
            PROPERTIES CXX_MODULE_STD ON
        )
    endif()
endforeach()
