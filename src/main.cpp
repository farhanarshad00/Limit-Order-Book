#include <iostream>
#include <list>
#include <unordered_map>
#include <map>
#include <fstream>
#include <chrono>
#include <ctime>
#include "Order.h"
#include "OrderBook.h"



int main() {
    OrderBook book;
    /*
    try {
        Order o1(1234, 50.25, 6, Side::BUY);
        book.addLimitOrder(o1);
    } catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    try {
        Order o2(9876, 75, 6, Side::SELL);
        book.addLimitOrder(o2);
    } catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    try {
        Order o3(3333, 50.25, 3, Side::BUY);
        book.addLimitOrder(o3);
    } catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    try {
        Order o4(4444, 74.00, 10, Side::SELL);
        book.addLimitOrder(o4);
    } catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    try {
        Order o5(5555, 76.00, 5, Side::SELL);
        book.addLimitOrder(o5);
    } catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    try {
        Order o6(6666, 49.00, 8, Side::BUY);
        book.addLimitOrder(o6);
    } catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    try {
        Order o7(7777, 80.00, 4, Side::BUY);
        book.addLimitOrder(o7);
    } catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    */
   /*
    book.addLimitOrder(Order(1, 100.0, 5, Side::BUY));
    book.addLimitOrder(Order(2, 100.0, 3, Side::BUY)); // same price level
    book.cancelOrder(1);
    book.print(); // should show only order 2 at 100.0

    // Test 2: cancel the last order at a price level
    book.cancelOrder(2);

    book.print();
    book.cancelOrder(9999); // should do nothing, not crash
    */
   
    std::ofstream tradeLog("trades.csv");
    tradeLog << "timestamp,buy_id,sell_id,price,quantity" << std::endl;
    // Set up a resting ask
    book.addLimitOrder(Order(1, 100.0, 10, Side::SELL));
    book.addLimitOrder(Order(2, 101.0, 5, Side::SELL));
    book.print();

    // Market buy for 8 shares — should fill against order 1 at 100.0
    book.addMarketOrder(Order(3, 0, 8, Side::BUY), tradeLog);
    book.print();
    // order 1 should have 2 shares remaining at 100.0
    // order 2 should be untouched at 101.0

    // Market buy for 20 shares — more than available
    book.addMarketOrder(Order(4, 0, 20, Side::BUY), tradeLog);
    book.print();
    // should fill order 1 remainder (2) and order 2 (5), total 7 shares
    // remainder of 13 should be cancelled, book should be empty
    book.matchOrders(tradeLog);
    tradeLog.close();

    std::cout << "\n--- AFTER MATCHING ---" << std::endl;
    book.print();

    return 0;
}