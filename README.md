# Limit-Order-Book


A C++17 limit order book matching engine implementing price-time priority for limit orders, market orders, cancels, and modifies with iterator based O(1) order lookup and removal, CSV trade logging, and a replay harness.

## Features
- [x] Limit orders with price-time priority
- [x] Market orders
- [x] Cancel — O(1) via iterator index
- [x] Modify — quantity decreases keep queue priority; price changes and increases re-queue
- [x] CSV trade logging
- [x] Catch2 unit tests (24 tests)
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
| Total time | 0.081 seconds |
| Throughput | ~1,238,267 orders/sec |
| p50 per-order latency | 125 ns |
| p99 per-order latency | ~3.4 μs |
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

The p99/p50 gap (~27x) reflects the tail cases where a match walks through multiple 
resting orders at a price level, rather than a single fast insert. Pre-sizing the 
order index with reserve() left p99 unchanged, ruling out hash-table rehashing as 
the cause. Note that p50 is only 2–3 ticks of the M1's ~41.7 ns timer, so it is 
accurate to about ±40 ns.

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
80758 microseconds
Total Time in Seconds: 0.080758
Throughput: 1238267 orders/sec
p50 latency: 125 ns
p99 latency: 3375 ns
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
                order_id -> OrderLocation {side, list iterator, level iterator}

```
**Note:** For cancel rows in the replay CSV, only the `id` 
column is used — the side, price, and quantity fields 
are ignored and can be set to 0.

### Bid Book
std::map sorted by highest price first, so the best bid is always begin() in O(1); adding a new price level costs O(log n). Pairs are stored as (key,value) -> (price, std::list<Order>)

### Ask Book
std::map sorted by lowest price first, so the best ask is always begin() in O(1); adding a new price level costs O(log n). Pairs are stored as (key,value) -> (price, std::list<Order>)

### Order Lookup Index
std::unordered_map due to O(1) average time complexity when inserting or deleting elements. Pairs are stored as (key,value) -> (order_id, OrderLocation). OrderLocation stores the order's side, a list iterator to the order, and an iterator to its price level. Cancels unlink the order and, if its level empties, erase the level by iterator, so cancel never searches the book (average-case O(1)).

## Design Decisions
- **Modify semantics**: A quantity decrease at the same price keeps queue priority, since it can't disadvantage orders behind it. A price change or size increase is a new commitment, so the order is cancelled and re-queued at the back, preventing traders from gaming their queue position.
- **Trades at the resting price**: Each trade executes at the price of the order already in the book, so an incoming buy lifts the ask and an incoming sell hits the bid.
- **Duplicate IDs rejected**: Adding an order with an existing ID throws, keeping the lookup index consistent with the book.   
- **Market orders extreme price**: Ensuring market orders are matched instantly to the highest bid/lowest ask depending on side, each market order's price was set at 0.0/std::numeric_limits<double>::max() to match orders using existing limit order infrastructure.
- **First in first out ordering**: Following the practice set by real order books that use FIFO ordering to ensure fairness and protect traders who committed earlier therefore incentivizing liquidity provision.

## What I Would Build Next
- **Lock based data structures**: Currently the engine has a vulnerability handling concurrent order submissions from multiple threads. In order to make the engine thread safe I would implement mutual exclusion to protect against data corruption in the book.
- **FIX Parser**: Real exchanges and brokers communicate using the FIX protocol standard. Adding a FIX parser would allow the engine to process real market data feeds rather than the custom CSV format currently used. 
- **Array based price structure**: Using std::map structures to store the price level along with other order information has worked for a benchmark of 100,000 orders. However, in order to scale up and increase the speed of the engine I would store the price levels in a fixed size array thereby ensuring significantly better cache performance and faster traversal.
