# Limit-Order-Book


A C++ 17 limit order book matching engine implementing price-time priority for limit orders, market orders, cancels, and modifies with iterator based O(1) order lookup and removal, CSV trade logging, and a replay harness.

## Build

**Requirements:** C++17 compiler, CMake 3.20+

```bash
git clone https://github.com/farhanarshad00/Limit-Order-Book.git
cd Limit-Order-Book
mkdir build && cd build
cmake ..
make
```

## Run

```bash
./orderbook
```

## Run Tests

```bash
./tests
```

Catch2 is fetched automatically by CMake on first build — no manual installation needed.
