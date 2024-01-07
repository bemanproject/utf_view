cmake_minimum_required(VERSION 3.28)
project(p2728 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_FORMAT_ENUMERATION Off)
option(WCHAR_T_IS_CODE_UNIT Off)
option(CHAR_IS_CODE_UNIT Off)
option(CHARN_T_POINTERS_ARE_RANGE_LIKE Off)

if (USE_FORMAT_ENUMERATION)
  add_definitions(-DUSE_FORMAT_ENUMERATION)
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
else()
  if (CHAR_IS_CODE_UNIT)
    message(FATAL_ERROR, "CHAR_IS_CODE_UNIT requires USE_FORMAT_ENUMERATION")
  endif()
  if (WCHAR_T_IS_CODE_UNIT)
    message(FATAL_ERROR, "WCHAR_T_IS_CODE_UNIT requires USE_FORMAT_ENUMERATION")
  endif()
endif()

if (CHARN_T_POINTERS_ARE_RANGE_LIKE)
  add_definitions(-DCHARN_T_POINTERS_ARE_RANGE_LIKE)
endif()

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
