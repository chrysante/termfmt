cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 20)

# download CPM.cmake
file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

project(
  termfmt
  VERSION 0.0.1
  DESCRIPTION ""
  LANGUAGES CXX)

set(CMAKE_CXX_VISIBILITY_PRESET     hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_library(termfmt)

include(GenerateExportHeader)
generate_export_header(termfmt
  EXPORT_MACRO_NAME TERMFMT_API
  EXPORT_FILE_NAME include/termfmt/api.h
)

target_include_directories(termfmt
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/include
        include
)

add_subdirectory(include/termfmt)
source_group(include/termfmt REGULAR_EXPRESSION "include/termfmt/*")

add_subdirectory(src)
source_group(src REGULAR_EXPRESSION "src/*")

if(NOT PROJECT_IS_TOP_LEVEL)
  return()
endif()

add_executable(test)
CPMAddPackage("gh:catchorg/Catch2@3.4.0")
target_include_directories(test
  PRIVATE
    include
    test
    ${Catch2_SOURCE_DIR}/src
)

target_link_libraries(test termfmt)
target_link_libraries(test Catch2::Catch2)
target_link_libraries(test Catch2::Catch2WithMain)
add_subdirectory(test)
source_group(test REGULAR_EXPRESSION "test/*")