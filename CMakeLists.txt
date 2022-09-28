cmake_minimum_required(VERSION 3.15)
project(BBST)

set(CMAKE_CXX_STANDARD 20)

enable_testing()
include(FetchContent)
FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1)

FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.7.0)

FetchContent_MakeAvailable(
        googletest
        googlebenchmark)

add_executable(exhaustive_testing tests/exhaustive_testing.cpp)
target_link_libraries(exhaustive_testing GTest::gtest_main)
target_compile_options(exhaustive_testing PRIVATE -g -fsanitize=address -fsanitize=undefined -O2)
target_link_options(exhaustive_testing PRIVATE -g -fsanitize=address -fsanitize=undefined -O2)

add_executable(stress_testing tests/stress_testing.cpp)
target_link_libraries(stress_testing GTest::gtest_main)
target_compile_definitions(stress_testing PRIVATE NDEBUG)
target_compile_options(stress_testing PRIVATE -g -O2)
target_link_options(stress_testing PRIVATE -g -O2)

include(GoogleTest)
gtest_discover_tests(exhaustive_testing)
gtest_discover_tests(stress_testing)

add_executable(benchmarkme benchmark/ benchmark/benchmark.cpp)
target_link_libraries(benchmarkme benchmark::benchmark)

add_library(bbst STATIC bbst.cpp)
target_compile_definitions(bbst PRIVATE NDEBUG)
target_compile_options(bbst PRIVATE -g -O2)
target_link_options(bbst PRIVATE -g -O2)
