cmake_minimum_required(VERSION 3.28)
project(p2728 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(thirdparty/null_term)

add_library(p2728)
target_sources(p2728
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    iface.cpp
)
target_link_libraries(p2728 PUBLIC null_term)
add_executable(p2728_test test.cpp)
add_test(NAME p2728_test COMMAND test)
target_link_libraries(p2728_test PRIVATE p2728)
