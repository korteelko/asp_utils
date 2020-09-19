cmake_minimum_required(VERSION 3.5)

set(MAIN_PROJECT "asp_utils")
project(${MAIN_PROJECT}-fulltests)

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(PROJECT_FULLTEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(FULLTEST_LIBRARIES "")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("${PROJECT_ROOT}/lib/gtest" "lib/gtest")
list(APPEND FULLTEST_LIBRARIES gtest gtest_main)

# code coverage
add_compile_options(-fprofile-arcs -ftest-coverage)
list(APPEND FULLTEST_LIBRARIES gcov)

find_package(Threads REQUIRED)
list(APPEND FULLTEST_LIBRARIES Threads::Threads)

include(${PROJECT_ROOT}/utils_source.cmake)
# include(GoogleTest)

# utils tests
add_executable(
  test_utils

  ${UTILS_SOURCE}
  ${PROJECT_FULLTEST_DIR}/test_utils.cpp
  ${PROJECT_FULLTEST_DIR}/test_logging.cpp
)
target_link_libraries(test_utils ${FULLTEST_LIBRARIES})
