cmake_minimum_required(VERSION 3.15)
project("LimitOrderBook_cpp")
find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(LimitOrderBook_cpp bindings.cpp)