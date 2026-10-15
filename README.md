# Limit-Order-Book


A C++17 limit order book matching engine implementing price-time priority for limit orders, market orders, cancels, and modifies with iterator based O(1) order lookup and removal, CSV trade logging, and a replay harness.

## Features
- [x] Limit orders with price-time priority
- [x] Market orders
- [x] Cancel — O(1) via iterator index
- [x] Modify — cancel and reinsert
- [x] CSV trade logging
- [x] Catch2 unit tests (14 tests)
- [x] CSV replay harness
- [x] Throughput benchmark
- [x] Per-order p50/p99 latency benchmark

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

## Benchmark Results

| Metric | Result |
|--------|--------|
| Orders processed | 100,000 |
| Total time | 0.083 seconds |
| Throughput | ~1,206,490 orders/sec |
| p50 per-order latency | 125 ns |
| p99 per-order latency | ~3.5 μs |
| Compiler flags | -O2 -Wall -Wextra |
| Hardware | Apple M1, 8GB RAM, macOS 26.3 |

Unoptimized build (-O0) produced ~557,000 orders/sec — 
the throughput improvement reflects compiler optimizations 
only, with no code changes.

Throughput here is lower than in earlier runs (~1.4M orders/sec) because this benchmark 
now takes a `steady_clock` timestamp before and after every order to compute per-order 
latency — that per-iteration timer call adds measurable overhead on its own. All figures 
above (throughput, p50, p99) come from the same instrumented run for consistency, rather 
than mixing timed and untimed results.

The p99/p50 gap (~28x) reflects the tail cases where a match walks through multiple 
resting orders at a price level, rather than a single fast insert.

## Sample run

```
=== Initial Book ===
--- ASK BOOK ---
Price: 80
ID: 6
Quantity: 3
Side: SELL

Price: 85
ID: 5
Quantity: 4
Side: SELL

Price: 90
ID: 4
Quantity: 5
Side: SELL

Price: 230
ID: 7
Quantity: 13
Side: SELL

--- BID BOOK ---
Price: 50.25
ID: 1
Quantity: 6
Side: BUY

Price: 25
ID: 3
Quantity: 4
Side: BUY

Price: 10
ID: 2
Quantity: 9
Side: BUY

=== After Cancelling Order 7 ===
--- ASK BOOK ---
Price: 80
ID: 6
Quantity: 3
Side: SELL

Price: 85
ID: 5
Quantity: 4
Side: SELL

Price: 90
ID: 4
Quantity: 5
Side: SELL

--- BID BOOK ---
Price: 50.25
ID: 1
Quantity: 6
Side: BUY

Price: 25
ID: 3
Quantity: 4
Side: BUY

Price: 10
ID: 2
Quantity: 9
Side: BUY

=== After Market Order ===
--- ASK BOOK ---
Price: 80
ID: 6
Quantity: 1
Side: SELL

Price: 85
ID: 5
Quantity: 4
Side: SELL

Price: 90
ID: 4
Quantity: 5
Side: SELL

--- BID BOOK ---
Price: 50.25
ID: 1
Quantity: 6
Side: BUY

Price: 25
ID: 3
Quantity: 4
Side: BUY

Price: 10
ID: 2
Quantity: 9
Side: BUY

=== Replay ===
--- ASK BOOK ---
Price: 95
ID: 2
Quantity: 1
Side: SELL

--- BID BOOK ---
Price: 90
ID: 1
Quantity: 4
Side: BUY

Price: 85
ID: 3
Quantity: 5
Side: BUY

=== Benchmark ===
82885 microseconds
Total Time in Seconds: 0.082885
Throughput: 1206490 orders/sec
p50 latency: 125 ns
p99 latency: 3500 ns
```
 
## Architecture
The three core data structures and how they interact:

```
Incoming Order
      |
      v
 addLimitOrder()
      |
      |-----> bid_book / ask_book (std::map)
      |         price -> std::list<Order>
      |
      |-----> order_lookup (std::unordered_map)
                order_id -> OrderLocation {price, side, iterator}

```
**Note:** For cancel rows in the replay CSV, only the `id` 
column is used — the side, price, and quantity fields 
are ignored and can be set to 0.

### Bid Book
std::map in order for pairs to be sorted by highest price level, resulting in O(log n) time complexity rather than O(n) complexity when locating highest bids. Pairs are stored as (key,value) -> (price, std::list<Order>)

### Ask Book
std::map in order for pairs to be sorted by lowest price level, resulting in O(log n) time complexity rather than O(n) complexity when locating lowest asks. Pairs are stored as (key,value) -> (price, std::list<Order>)

### Order Lookup Index
std::unordered_map due to O(1) time complexity when inserting or deleting elements. Pairs are stored as (key,value) -> (order_id, OrderLocation). OrderLocation stores the price level, side, and a direct list iterator to the order, enabling O(1) cancel without scanning the book.

## Design Decisions
- **Cancel and reinsert for modified orders**: To protect against traders attempting to game the book, queue position is reset when traders make a new commitment.   
- **Market orders extreme price**: Ensuring market orders are matched instantly to the highest bid/lowest ask depending on side, each market order's price was set at 0.0/std::numeric_limits<double>::max() to match orders using existing limit order infrastructure.
- **First in first out ordering**: Following the practice set by real order books that use FIFO ordering to ensure fairness and protect traders who committed earlier therefore incentivizing liquidity provision.

## What I Would Build Next
- **Lock based data structures**: Currently the engine has a vulnerability handling concurrent order submissions from multiple threads. In order to make the engine thread safe I would implement mutual exclusion to protect against data corruption in the book.
- **FIX Parser**: Real exchanges and brokers communicate using the FIX protocol standard. Adding a FIX parser would allow the engine to process real market data feeds rather than the custom CSV format currently used. 
- **Array based price structure**: Using std::map structures to store the price level along with other order information has worked for a benchmark of 100,000 orders. However, in order to scale up and increase the speed of the engine I would store the price levels in a fixed size array thereby ensuring significantly better cache performance and faster traversal.
