cmake_minimum_required(VERSION 3.20)
project(rv32i_perf_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SIM_BUILD_TESTS "Build GoogleTest unit tests" ON)

add_library(sim_core
  src/decoder.cpp
  src/regfile.cpp
  src/alu.cpp
  src/memory.cpp
)
target_include_directories(sim_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(sim_core PRIVATE
  $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall -Wextra -Wpedantic>
)

add_executable(sim src/main.cpp)
target_link_libraries(sim PRIVATE sim_core)

if(SIM_BUILD_TESTS)
  enable_testing()
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  add_executable(sim_tests
    tests/test_decoder_formats.cpp
    tests/test_regfile.cpp
    tests/test_alu.cpp
    tests/test_memory.cpp
  )
  target_link_libraries(sim_tests PRIVATE sim_core GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(sim_tests)
endif()
