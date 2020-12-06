cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME readers_example)
project(${PROJECT_NAME})
set(target_exec ${PROJECT_NAME})

set(READERS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

if(WITH_PUGIXML AND WITH_RAPIDJSON)
  add_executable(${target_exec} main.cpp)
  target_compile_definitions(${target_exec} PUBLIC -D_DEBUG)
  target_link_libraries(${target_exec} asp_utils)
endif()
