cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME asp_utils)
project(${PROJECT_NAME})
set(TARGET_UTILS_LIB ${PROJECT_NAME})

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(MODULES_DIR ${PROJECT_ROOT}/lib)

message(STATUS "${PROJECT_NAME} build type: ${CMAKE_BUILD_TYPE}")

option(WITH_PUGIXML "Add `pugixml` api and examples" ON)
option(WITH_RAPIDJSON "Add `rapidjson` api and examples" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_TESTS "Build tests" OFF)

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
  PUBLIC ${PROJECT_ROOT}/include
  PRIVATE ${MODULES_DIR}/spdlog/include)

#   pugixml
#   https://pugixml.org/
if(WITH_PUGIXML)
  set(PUGIXML_DIR ${MODULES_DIR}/pugixml)
  target_compile_definitions(${TARGET_UTILS_LIB} PUBLIC PUGIXML_HEADER_ONLY WITH_PUGIXML)
  target_include_directories(${TARGET_UTILS_LIB} PUBLIC ${PUGIXML_DIR}/src)
  add_subdirectory(${PUGIXML_DIR})
endif()

#   rapidjson
#   http://rapidjson.org/index.html
if(WITH_RAPIDJSON)
  set(RAPIDJSON_DIR ${MODULES_DIR}/rapidjson)
  # !note: cannot build googletest by source(cmake error)
  set(RAPIDJSON_BUILD_THIRDPARTY_GTEST OFF CACHE BOOL "Переопределить опцию сборки googletest" FORCE)
  set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "Переопределить опцию сборки тестов" FORCE)
  target_include_directories(${TARGET_UTILS_LIB} PUBLIC ${RAPIDJSON_DIR}/include)
  target_compile_definitions(${TARGET_UTILS_LIB} PUBLIC WITH_RAPIDJSON)
  add_subdirectory(${RAPIDJSON_DIR})
endif()


# build type
if(CMAKE_BUILD_TYPE MATCHES Debug)
  target_compile_definitions(${TARGET_UTILS_LIB} PRIVATE -D_DEBUG)
endif()

find_package(Threads REQUIRED)
target_link_libraries(${TARGET_UTILS_LIB}
  Threads::Threads
  stdc++fs
  spdlog
)

# add examples
if(BUILD_EXAMPLES)
  message(STATUS "Собираем примеры")
  add_subdirectory(${PROJECT_ROOT}/examples)
endif()

# run tests
if(BUILD_TESTS)
  message(STATUS "Запускаем тесты ${PROJECT_NAME}")
  enable_testing()
  add_subdirectory(${PROJECT_ROOT}/tests)
endif()

