cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME asp_utils)
project(${PROJECT_NAME})
set(TARGET_UTILS_LIB ${PROJECT_NAME})

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(MODULES_DIR ${PROJECT_ROOT}/lib)

message(STATUS "${PROJECT_NAME} build type: ${CMAKE_BUILD_TYPE}")
option(TESTS_ENABLED "${PROJECT_NAME} option `TESTS_ENABLED`" OFF)

add_library(${TARGET_UTILS_LIB}
  ${PROJECT_ROOT}/source/Common.cpp
  ${PROJECT_ROOT}/source/ErrorWrap.cpp
  ${PROJECT_ROOT}/source/FileURL.cpp
  ${PROJECT_ROOT}/source/Logging.cpp
)

target_compile_features(${TARGET_UTILS_LIB} PUBLIC cxx_std_17)
#   spdlog
#   https://github.com/gabime/spdlog.git
add_subdirectory(${MODULES_DIR}/spdlog)
target_include_directories(${TARGET_UTILS_LIB}
  PUBLIC ${PROJECT_ROOT}/source
  PRIVATE ${MODULES_DIR}/spdlog/include
)

# build type
if(CMAKE_BUILD_TYPE MATCHES Debug)
  target_compile_definitions(${TARGET_UTILS_LIB} PRIVATE -DBYCMAKE_DEBUG)
endif()

find_package(Threads REQUIRED)

target_link_libraries(${TARGET_UTILS_LIB}
  Threads::Threads
  stdc++fs
  spdlog
)

# run tests
if(TESTS_ENABLED)
  message(STATUS "Запускаем тесты ${PROJECT_NAME}")
  enable_testing()
  add_subdirectory("${PROJECT_ROOT}/tests")
endif()
