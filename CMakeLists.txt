cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME asp_utils)
project(${PROJECT_NAME})

set(TARGET_LIB ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(MODULES_DIR ${PROJECT_ROOT}/lib)

# ==============================================================================
# OPTIONS
# ==============================================================================
option(TESTS_ENABLED "Run tests" OFF)

# build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DBYCMAKE_DEBUG)
endif()
message(STATUS "${PROJECT_NAME} build type: ${CMAKE_BUILD_TYPE}")

# ==============================================================================
# SOURCE
# ==============================================================================
set(UTILS_SOURCE
  ${PROJECT_ROOT}/source/Common.cpp
  ${PROJECT_ROOT}/source/ErrorWrap.cpp
  ${PROJECT_ROOT}/source/FileURL.cpp
  ${PROJECT_ROOT}/source/Logging.cpp
)

# ==============================================================================
# TARGET SETUP
# ==============================================================================
add_library(${TARGET_LIB} STATIC ${UTILS_SOURCE})

# ==============================================================================
# LIBRARIES AND MODULES
# ==============================================================================
#   spdlog
#   https://github.com/gabime/spdlog.git
add_subdirectory(${MODULES_DIR}/spdlog)
target_include_directories(${TARGET_LIB} PRIVATE ${MODULES_DIR}/spdlog/include)

target_include_directories(${TARGET_LIB} PRIVATE ${PROJECT_ROOT}/source)

target_compile_options(${TARGET_LIB} PRIVATE -Wall)
#   thread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(${TARGET_LIB}

  stdc++fs
  spdlog
  Threads::Threads
)

# run tests
if(TESTS_ENABLED)
  message(STATUS "Запускаем тесты")
  enable_testing()
  add_subdirectory("${PROJECT_ROOT}/tests")
endif()

