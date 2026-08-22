#include <iostream>
#include <pybind11/pybind11.h>

int add(int a, int b) {
    return a + b;   
}

PYBIND11_MODULE(LimitOrderBook_cpp, m) {
    m.def("add", &add, "adds a to b");
}