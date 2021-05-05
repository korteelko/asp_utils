set(TARGET_UTILS_TESTS asp_utils-fulltests)

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(PROJECT_FULLTEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})

find_package(GTest)
if(${GTEST_FOUND})
  # utils tests
  add_executable(${TARGET_UTILS_TESTS}
    ${PROJECT_ROOT}/source/Common.cpp
    ${PROJECT_ROOT}/source/ErrorWrap.cpp
    ${PROJECT_ROOT}/source/Logging.cpp
    ${PROJECT_FULLTEST_DIR}/test_utils.cpp
    ${PROJECT_FULLTEST_DIR}/test_logging.cpp
    ${PROJECT_FULLTEST_DIR}/test_nullobject.cpp
  )
  if(WIN32)
    target_compile_definitions(${TARGET_UTILS_TESTS} PRIVATE OS_WINDOWS)
  elseif(UNIX)
    target_compile_definitions(${TARGET_UTILS_TESTS} PRIVATE OS_UNIX)
  endif()
  target_compile_features(${TARGET_UTILS_TESTS} PRIVATE cxx_std_20)
  target_compile_definitions(${TARGET_UTILS_TESTS} PRIVATE -DBYCMAKE_DEBUG)
  target_compile_options(${TARGET_UTILS_TESTS} PRIVATE
      -fprofile-arcs -ftest-coverage -fconcepts -Wall)

  target_include_directories(${TARGET_UTILS_TESTS}
    PRIVATE ${PROJECT_ROOT}/include
    PRIVATE ${PROJECT_ROOT}/lib/spdlog/include
    PRIVATE ${GTEST_INCLUDE_DIRS}
  )

  find_package(Threads REQUIRED)
  target_link_libraries(${TARGET_UTILS_TESTS}
    Threads::Threads
    gtest gtest_main
    gcov
  )
endif(${GTEST_FOUND})
