cmake_minimum_required(VERSION 3.5)

set(MAIN_PROJECT "asp_utils")
project(${MAIN_PROJECT}-fulltests)
set(TARGET_UTILS_TESTS ${PROJECT_NAME})

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(PROJECT_FULLTEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(FULLTEST_LIBRARIES "")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(GTest)
if(${GTEST_FOUND})
  # utils tests
  add_executable(
    ${TARGET_UTILS_TESTS}

    ${UTILS_SOURCE}
    ${PROJECT_FULLTEST_DIR}/test_utils.cpp
    ${PROJECT_FULLTEST_DIR}/test_logging.cpp
  )
  target_include_directories(${TARGET_UTILS_TESTS} PRIVATE ${PROJECT_ROOT}/source)
  target_include_directories(${TARGET_UTILS_TESTS} PRIVATE ${PROJECT_ROOT}/lib/spdlog/include)
  target_include_directories(${TARGET_UTILS_TESTS} PRIVATE ${GTEST_INCLUDE_DIRS})
  target_compile_options(${TARGET_UTILS_TESTS} PRIVATE -fprofile-arcs
      -ftest-coverage -Wall)

  find_package(Threads REQUIRED)
  list(APPEND FULLTEST_LIBRARIES Threads::Threads)

  list(APPEND FULLTEST_LIBRARIES gtest gtest_main)
  # code coverage
  list(APPEND FULLTEST_LIBRARIES gcov)

  target_link_libraries(${TARGET_UTILS_TESTS} PRIVATE ${FULLTEST_LIBRARIES})
endif(${GTEST_FOUND})
