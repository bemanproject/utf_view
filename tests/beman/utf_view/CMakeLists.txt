# SPDX-License-Identifier: BSL-1.0

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(BEMAN_UTF_VIEW_BUILD_TESTS)
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
endif()

if(BEMAN_UTF_VIEW_BUILD_BENCHMARKS)
    set(BENCHMARK_ENABLE_TESTING OFF)
    find_package(benchmark REQUIRED)

    include(FetchContent)
    FetchContent_Declare(
        unicode_lipsum
        GIT_REPOSITORY https://github.com/lemire/unicode_lipsum.git
        GIT_TAG b0f1d0c1cb0cb168fc08dbf0e3b7100cdec517dc
    )
    # unicode_lipsum is a data-only repository with no CMakeLists.txt;
    # MakeAvailable just populates the source tree for us to read at runtime.
    FetchContent_MakeAvailable(unicode_lipsum)

    add_executable(beman.utf_view.benchmarks.to_utf to_utf.benchmark.cpp)
    target_link_libraries(
        beman.utf_view.benchmarks.to_utf
        PRIVATE beman::utf_view benchmark::benchmark
    )
    target_compile_definitions(
        beman.utf_view.benchmarks.to_utf
        PRIVATE
            "UNICODE_LIPSUM_DIR=\"${unicode_lipsum_SOURCE_DIR}/lipsum\""
    )

    if(NOT TARGET beman.utf_view.benchmarks)
        add_custom_target(beman.utf_view.benchmarks)
    endif()
    add_dependencies(
        beman.utf_view.benchmarks
        beman.utf_view.benchmarks.to_utf
    )
    if(BEMAN_UTF_VIEW_USE_MODULES)
        set_target_properties(beman.utf_view.benchmarks.to_utf PROPERTIES CXX_MODULE_STD ON)
    endif()
endif()
