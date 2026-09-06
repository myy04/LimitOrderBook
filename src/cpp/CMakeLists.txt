cmake_minimum_required(VERSION 3.15)
project(LimitOrderBook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# turn off optimizations
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


add_library(lob_core STATIC
    ./src/MatchingEngine.cpp
    ./src/OrderBook.cpp
    ./src/OrderGenerator.cpp
    ./src/SnapshotBuffer.cpp
    ./src/OrderGateway.cpp
)

target_include_directories(lob_core PUBLIC ./include)

add_subdirectory(./benchmark)
