cmake_minimum_required(VERSION 3.10)

option(BUILD_BENCHMARKS "Build Benchmarks" ON)

if (BUILD_BENCHMARKS) 

    find_package(benchmark REQUIRED)

    add_executable(benchmark_cpp benchmark.cpp)

    target_link_libraries(benchmark_cpp PRIVATE lob_core)
    target_link_libraries(benchmark_cpp PRIVATE benchmark::benchmark)
endif()
