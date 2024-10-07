# SPDX-License-Identifier: GPL-3.0-only

add_library(utfview INTERFACE)

include(GNUInstallDirs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(
  utfview
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_LOWER_PROJECT_NAME}> # <prefix>/include/utfview
)

target_link_libraries(utfview INTERFACE boost_stl_interfaces)

install(
  TARGETS utfview
  EXPORT ${TARGETS_EXPORT_NAME}
  DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME}
  FILES_MATCHING
  PATTERN "*.hpp")

# Tests
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
