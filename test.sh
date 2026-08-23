#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

# Builds and runs the project's test suites.
#
# Usage:
#   ./test.sh          # run both the C++ (GoogleTest) and Python (pytest) suites
#   ./test.sh cpp      # run only the C++ GoogleTest suite
#   ./test.sh py        # run only the Python pytest suite

MODE="${1:-all}"

run_cpp_tests() {
  echo "==> Configuring and building C++ tests (GoogleTest)"
  cmake -S src/cpp -B build-tests \
    -DCMAKE_PREFIX_PATH="$(
      .venv/bin/python3 -c 'import pybind11; print(pybind11.get_cmake_dir())'
    )" \
    -DLOB_BUILD_TESTS=ON

  if ! cmake --build build-tests --target lob_tests -j2; then
    echo "Failed to build lob_tests. Is GoogleTest installed? (e.g. 'brew install googletest')" >&2
    exit 1
  fi

  echo "==> Running C++ test suite"
  ./build-tests/tests/lob_tests
}

run_python_tests() {
  echo "==> Running Python test suite (pytest)"
  .venv/bin/python3 -m pytest
}

case "$MODE" in
  all)
    run_cpp_tests
    run_python_tests
    ;;
  cpp)
    run_cpp_tests
    ;;
  py)
    run_python_tests
    ;;
  *)
    echo "Unknown mode: $MODE (expected 'all', 'cpp', or 'py')" >&2
    exit 1
    ;;
esac
