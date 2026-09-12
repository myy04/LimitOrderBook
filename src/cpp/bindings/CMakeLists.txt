cmake_minimum_required(VERSION 3.10)

find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(LimitOrderBook_cpp ./bindings.cpp)
target_link_libraries(LimitOrderBook_cpp PRIVATE lob_core)
target_include_directories(LimitOrderBook_cpp PRIVATE ${CMAKE_SOURCE_DIR}/include)

if(NOT SKBUILD)
    set_target_properties(LimitOrderBook_cpp PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../python/LimitOrderBook
    )
endif()
# Install destinations are relative to site-packages.
install(TARGETS LimitOrderBook_cpp DESTINATION LimitOrderBook)