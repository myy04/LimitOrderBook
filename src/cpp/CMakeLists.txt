cmake_minimum_required(VERSION 3.15)
project("LimitOrderBook_cpp")
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 REQUIRED)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "../src/LimitOrderBook")
set(CMAKE_CXX_STANDARD_REQUIRED 20)

pybind11_add_module(LimitOrderBook_cpp 
    bindings.cpp
    ./src/OrderBook.cpp
    ./src/MatchingEngine.cpp
    ./src/SnapshotBuffer.cpp
)

option(LOB_BUILD_TESTS "Build the C++ unit tests (requires GoogleTest)" ON)
if(LOB_BUILD_TESTS)
    find_package(GTest QUIET)
    if(GTest_FOUND)
        enable_testing()
        add_subdirectory(tests)
    else()
        message(STATUS "GoogleTest not found; skipping LimitOrderBook_cpp tests")
    endif()
endif()