cmake_minimum_required(VERSION 3.10)

find_package(benchmark REQUIRED)

add_executable(benchmark_cpp benchmark.cpp)

target_link_libraries(benchmark_cpp PRIVATE lob_core_pure)
target_link_libraries(benchmark_cpp PRIVATE benchmark::benchmark)

add_executable(sample sample.cpp)
target_link_libraries(sample PRIVATE lob_core)
