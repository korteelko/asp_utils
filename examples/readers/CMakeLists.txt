cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME readers_example)
project(${PROJECT_NAME})
set(target_exec ${PROJECT_NAME})

set(READERS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

add_executable(${target_exec} main.cpp)

target_compile_features(${target_exec} PUBLIC cxx_std_17)

set(PUGIXML_DIR ${READERS_ROOT}/lib/pugixml)
add_subdirectory(${PUGIXML_DIR} lib/pugixml)

set(RAPIDJSON_DIR ${READERS_ROOT}/lib/rapidjson)
add_subdirectory(${RAPIDJSON_DIR} lib/rapidjson)

target_include_directories(${target_exec} PUBLIC ${RAPIDJSON_DIR}/include ${PUGIXML_DIR}/src)
target_compile_definitions(${target_exec} PUBLIC -D_DEBUG -DWITH_PUGIXML -DWITH_RAPIDJSON)

target_link_libraries(${target_exec} pugixml asp_utils)

