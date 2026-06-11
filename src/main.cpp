#include <iostream>
#include <fstream>
#include "Order.h"
#include "OrderBook.h"
#include "Replay.h"
#include "Benchmark.h"

int main() {
    OrderBook book;
    
    // Adding Limit Orders
    book.addLimitOrder(Order(1, 50.25, 6, Side::BUY));
    book.addLimitOrder(Order(2, 10, 9, Side::BUY));
    book.addLimitOrder(Order(3, 25, 4, Side::BUY));
    book.addLimitOrder(Order(4, 90, 5, Side::SELL));
    book.addLimitOrder(Order(5, 85, 4, Side::SELL));
    book.addLimitOrder(Order(6, 80, 3, Side::SELL));
    book.addLimitOrder(Order(7, 230, 13, Side::SELL));
    std::cout << "=== Initial Book ===" << std::endl;
    book.print();

    std::cout << "=== After Cancelling Order 7 ===" << std::endl;
    // Testing cancelOrder, book should be same with no order 7
    book.cancelOrder(7);
    book.print();

    // Adding Market Order to buy 2 shares should leave order 6 with 1 share left
    std::ofstream tradeLog("trades.csv");
    book.addMarketOrder(Order(8, 0, 2, Side::BUY), tradeLog);
    std::cout << "=== After Market Order ===" << std::endl;
    book.print();

    // Testing Replay Harness
    std::cout << "\n=== Replay ===" << std::endl;
    OrderBook replayBook;
    std::ofstream replayLog("replay_trades.csv");
    replayLog << "timestamp,buy_id,sell_id,price,quantity\n";
    Replay::run(replayBook, "../data/sample_orders.csv", replayLog);
    replayBook.print();

    // Testing Benchmark with 100,000 simulated orders
    std::cout << "\n=== Benchmark ===" << std::endl;
    Benchmark::run(100000);
}