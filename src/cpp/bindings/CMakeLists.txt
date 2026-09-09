cmake_minimum_required(VERSION 3.10)

option(BUILD_BINDINGS "Build bindings" ON)

if (BUILD_BINDINGS)
    find_package(pybind11 CONFIG REQUIRED)
    pybind11_add_module(LimitOrderBook_cpp ./bindings.cpp)
    target_link_libraries(LimitOrderBook_cpp PRIVATE lob_core)
    target_include_directories(LimitOrderBook_cpp PRIVATE ${CMAKE_SOURCE_DIR}/include)
    # Drop the compiled module inside the Python package so that
    # "import LimitOrderBook" resolves the native extension directly.
    set_target_properties(LimitOrderBook_cpp PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../python/LimitOrderBook
    )
    install(TARGETS LimitOrderBook_cpp DESTINATION python/LimitOrderBook)
endif()