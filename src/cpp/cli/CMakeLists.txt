cmake_minimum_required(VERSION 3.10)

option(BUILD_CLI "Build CLI" ON)

if (BUILD_CLI) 
    add_executable(cli cli.cpp)
    target_link_libraries(cli PRIVATE lob_core)
endif()