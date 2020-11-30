cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME readers_example)
project(${PROJECT_NAME})
set(target_exec ${PROJECT_NAME})

set(READERS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

add_executable(${target_exec} main.cpp)

target_compile_features(${target_exec} PUBLIC cxx_std_17)
target_compile_definitions(${target_exec} PUBLIC -D_DEBUG -DOS_NIX)

# include_directories("utils")

set(PUGIXML_DIR "${READERS_ROOT}/lib/pugixml")
add_subdirectory(${PUGIXML_DIR} "lib/pugixml")
include_directories("${PUGIXML_DIR}/src")
target_compile_definitions(${target_exec} PUBLIC -DWITH_PUGIXML)

set(RAPIDJSON_DIR "${READERS_ROOT}/lib/rapidjson")
add_subdirectory(${RAPIDJSON_DIR} "lib/rapidjson")
include_directories("${RAPIDJSON_DIR}/include")
target_compile_definitions(${target_exec} PUBLIC -DWITH_RAPIDJSON)

# rapidjson used as header only
target_link_libraries(${target_exec} pugixml asp_utils)

