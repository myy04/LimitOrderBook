#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

# Builds the LimitOrderBook_cpp Python extension module.
# Tests are excluded here so a plain build doesn't require GoogleTest;
# use ./test.sh to build and run the test suites.

rm -rf build

cmake -S src/cpp -B build \
  -DCMAKE_PREFIX_PATH="$(
    .venv/bin/python3 -c 'import pybind11; print(pybind11.get_cmake_dir())'
  )" \
  -DCMAKE_BUILD_TYPE=Release \
  -DLOB_BUILD_TESTS=OFF

cmake --build build --config Release -j2

# PYTHONPATH=build .venv/bin/python3 -c \
# 'import LimitOrderBook_cpp; print("C++ module loaded")'

PYTHONPATH=./build