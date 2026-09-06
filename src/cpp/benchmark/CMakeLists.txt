cmake_minimum_required(VERSION 3.10)

option(BUILD_BENCHMARKS "Build Benchmarks" ON)

option(BUILD_SAMPLE "Build sample" ON)

if (BUILD_BENCHMARKS) 

    find_package(benchmark REQUIRED)

    add_executable(benchmark_cpp benchmark.cpp)

    target_link_libraries(benchmark_cpp PRIVATE lob_core)
    target_link_libraries(benchmark_cpp PRIVATE benchmark::benchmark)
endif()

if (BUILD_SAMPLE)
    add_executable(sample sample.cpp)
    target_link_libraries(sample PRIVATE lob_core)
endif()