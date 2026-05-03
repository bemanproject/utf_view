# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

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
