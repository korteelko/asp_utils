cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME asp_utils)
project(${PROJECT_NAME})


add_compile_options(-Wall)

set(CMAKE_BUILD_TYPE Debug)
add_definitions(-DBYCMAKE_DEBUG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-DBYCMAKE_CXX17)

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(MODULES_DIR ${PROJECT_ROOT}/lib)

if(UNIX)
  add_definitions(-DOS_NIX)
endif(UNIX)

# OPTIONS
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  option(BUILD_EXAMPLES "Build examples" ON)
  option(TESTS_ENABLED "Run tests" ON)
endif()

#======================================================
# SOURCE
#======================================================
set(UTILS_SOURCE
  ${PROJECT_ROOT}/source/Common.cpp
  ${PROJECT_ROOT}/source/ErrorWrap.cpp
  ${PROJECT_ROOT}/source/FileURL.cpp
  ${PROJECT_ROOT}/source/Logging.cpp
)
include_directories(${PROJECT_ROOT}/source)

#======================================================
# LIBRARIES AND MODULES
#======================================================
#   spdlog
#   https://github.com/gabime/spdlog.git
add_subdirectory(${MODULES_DIR}/spdlog)
include_directories(${MODULES_DIR}/spdlog/include)


#   thread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#======================================================
# OUTPUT SETUP
#======================================================
add_library(${PROJECT_NAME} STATIC ${DATABASE_SOURCE} ${UTILS_SOURCE})

target_link_libraries(${PROJECT_NAME}

  stdc++fs
  spdlog
  Threads::Threads
)

# run tests
if(TESTS_ENABLED)
  message(STATUS "Запускаем тесты")
  enable_testing()
  include(GoogleTest)
  add_subdirectory("${PROJECT_ROOT}/tests")
endif()
