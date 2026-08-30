# Limit Order Book (LOB)

A high-performance Limit Order Book implementation featuring both Python and C++ cores, designed for efficient order matching and market data snapshotting.

## 🚀 Features

- **Dual Implementation**: Core logic implemented in both Python (for flexibility) and C++ (for performance) with Pybind11 bindings.
- **Efficient Matching**: Optimized matching engine to handle buy and sell orders.
- **Order Book Management**: Supports insertion, removal, and priority-based order tracking using doubly linked lists and sorted dictionaries.
- **Real-time Snapshots**: Snapshot buffer mechanism to provide a view of the best bids and asks (market depth).
- **CLI Visualizer**: A command-line interface to monitor the order book, spread, and current market state in real-time.

## 🛠️ Tech Stack

- **Languages**: Python, Modern C++
- **Build System**: CMake
- **Bindings**: Pybind11
- **Testing**: GoogleTest (C++)
- **Dependencies**: `sortedcontainers` (Python)

## 📁 Project Structure

- `src/LimitOrderBook/`: Python implementation of the order book and matching engine.
- `src/cpp/`: High-performance C++ implementation.
- `src/CLI.py`: Command-line interface for interacting with the engine.
- `src/OrderGenerator.py`: Utility to simulate order flow.

## ⚙️ Installation & Setup

### Prerequisites
- Python 3.x
- CMake 3.15+
- C++ compatible compiler (GCC, Clang, or MSVC)
- Pybind11

### Building the Project
Use the provided build script to compile the C++ core and Python extensions:
```bash
chmod +x build.sh
./build.sh
```

### Python Dependencies
```bash
pip install -r requirements.txt
```

## 🚀 Usage

### Running the Simulation
You can run the simulation using either the Python or C++ engine:

**Using the Python engine:**
```bash
python run.py python
```

**Using the C++ engine:**
```bash
python run.py cpp
```

### Performance Benchmarking
Compare the performance of the Python and C++ engines using the benchmark tool:
```bash
python benchmark.py
```

## 🧪 Testing
Use the provided test script to run the C++ (GoogleTest) and Python (pytest) suites:
```bash
chmod +x test.sh
./test.sh          # Run all tests
./test.sh cpp      # Run C++ tests only
./test.sh py       # Run Python tests only
```

## CLI Screenshot

![CLI Screenshot](./images/cli_screenshot.png)