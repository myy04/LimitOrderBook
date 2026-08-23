rm -rf build

cmake -S src/cpp -B build \
  -DCMAKE_PREFIX_PATH="$(
    .venv/bin/python3 -c 'import pybind11; print(pybind11.get_cmake_dir())'
  )" \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release -j2

PYTHONPATH=build .venv/bin/python3 -c \
'import LimitOrderBook_cpp; print("C++ module loaded")'
