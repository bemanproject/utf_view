cmake_minimum_required(VERSION 3.28)
project(p2728 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(WCHAR_T_IS_CODE_UNIT Off)
option(CHAR_IS_CODE_UNIT Off)
option(CHARN_T_POINTERS_ARE_RANGE_LIKE Off)

if (WCHAR_T_IS_CODE_UNIT)
  add_definitions(-DWCHAR_T_IS_CODE_UNIT)
  if (CHAR_IS_CODE_UNIT)
    add_definitions(-DCHAR_IS_CODE_UNIT)
  endif()
else()
  if (CHAR_IS_CODE_UNIT)
    message(FATAL_ERROR, "CHAR_IS_CODE_UNIT requires WCHAR_T_IS_CODE_UNIT")
  endif()
endif()

if (CHARN_T_POINTERS_ARE_RANGE_LIKE)
  add_definitions(-DCHARN_T_POINTERS_ARE_RANGE_LIKE)
endif()

if (NOT TARGET null_term)
  add_subdirectory(thirdparty/null_term)
endif()

if (NOT TARGET std_archetypes)
  add_subdirectory(thirdparty/std_archetypes)
endif()

set(P2728_INPUT_FILES
     ${CMAKE_SOURCE_DIR}/concepts.cpp.in
     ${CMAKE_SOURCE_DIR}/concepts_test.cpp.in
     ${CMAKE_SOURCE_DIR}/utf_iterator.cpp.in
     ${CMAKE_SOURCE_DIR}/utf_iterator_test.cpp.in)

set(P2728_FILE_SET
    ${CMAKE_CURRENT_BINARY_DIR}/concepts.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/utf_iterator.cpp)

set(P2728_TEST_FILE_SET
    ${CMAKE_CURRENT_BINARY_DIR}/concepts_test.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/utf_iterator_test.cpp)

set(P2728_LIB_DEPS
    null_term std_archetypes)

add_custom_command(
  OUTPUT ${P2728_FILE_SET} ${P2728_TEST_FILE_SET}
  DEPENDS ${CMAKE_SOURCE_DIR}/preprocess.py ${P2728_INPUT_FILES}
  COMMAND python3 ${CMAKE_SOURCE_DIR}/preprocess.py
  ARGS --out-dir ${CMAKE_CURRENT_BINARY_DIR} ${P2728_INPUT_FILES})

add_library(p2728)
target_sources(p2728
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
   iface.cpp
   ${P2728_FILE_SET}
)
target_link_libraries(p2728 PUBLIC ${P2728_LIB_DEPS})
add_library(p2728_test)
target_sources(p2728_test
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
   iface.cpp
   ${P2728_FILE_SET}
   ${P2728_TEST_FILE_SET}
)
target_link_libraries(p2728_test PUBLIC ${P2728_LIB_DEPS})
add_executable(test_p2728 test.cpp)
target_link_libraries(test_p2728 PRIVATE p2728_test)
