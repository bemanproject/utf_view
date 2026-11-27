# SPDX-License-Identifier: BSL-1.0

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
target_include_directories(
    beman_utf_view_test_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(beman_utf_view_test main.test.cpp)

target_link_libraries(
    beman_utf_view_test
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,beman_utf_view_test_lib>"
)

add_test(NAME beman_utf_view_test COMMAND beman_utf_view_test)

if(BEMAN_UTF_VIEW_USE_MODULES AND CMAKE_VERSION VERSION_LESS "4.5.0")
    set_target_properties(beman_utf_view_test_lib PROPERTIES CXX_MODULE_STD ON)
    set_target_properties(beman_utf_view_test PROPERTIES CXX_MODULE_STD ON)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    return()
endif()

if(CMAKE_VERSION VERSION_LESS "4.5.0")
    return() # not if CMAKE_EXPERIMENTAL_CXX_IMPORT_STD needed! CK
endif()

#======================================================================
# NOTE: Only for Release build with enabled install rules useful! CK
#======================================================================
if(BEMAN_UTF_VIEW_INSTALL_CONFIG_FILE_PACKAGE AND NOT CMAKE_SKIP_INSTALL_RULES)
    # test if the targets are usable from the install directory
    add_test(
        NAME install-to-stagedir
        COMMAND
            ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix
            ${CMAKE_BINARY_DIR}/stagedir --config $<CONFIG>
    )
    add_test(
        NAME find-package-test
        COMMAND
            ${CMAKE_CTEST_COMMAND} # --verbose
            --output-on-failure -C $<CONFIG> #
            --build-and-test "${CMAKE_SOURCE_DIR}/examples"
            "${CMAKE_CURRENT_BINARY_DIR}/find-package-test" #
            --build-generator ${CMAKE_GENERATOR} #
            --build-makeprogram ${CMAKE_MAKE_PROGRAM} #
            --build-options #
            "-D BEMAN_BUILDSYS_SANITIZER=${BEMAN_BUILDSYS_SANITIZER}"
            "-D BEMAN_UTF_VIEW_USE_MODULES=${BEMAN_UTF_VIEW_USE_MODULES}"
            "-D CMAKE_BUILD_TYPE=$<CONFIG>"
            "-D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
            "-D CMAKE_CXX_EXTENSIONS=${CMAKE_CXX_EXTENSIONS}"
            "-D CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}"
            "-D CMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}"
            "-D CMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/stagedir"
            "-D CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
    )
endif()
