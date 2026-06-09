#include <catch2/catch_test_macros.hpp>
#include "OrderBook.h"
#include <fstream>

TEST_CASE("Limit order inserts into bid book") {
    OrderBook book;
    book.addLimitOrder(Order(1, 100.0, 5, Side::BUY));
    REQUIRE(book.getBidBook().count(100.0) == 1);
}

TEST_CASE("Bid Book sorted highest price first") {
    OrderBook book;
    book.addLimitOrder(Order(1, 100.0, 5, Side::BUY));
    book.addLimitOrder(Order(2, 200.0, 6, Side::BUY));
    book.addLimitOrder(Order(3, 300.0, 7, Side::BUY));
    REQUIRE(book.getBidBook().begin()->first == 300.0);
}

TEST_CASE("Ask Book sorted lowest price first") {
    OrderBook book;
    book.addLimitOrder(Order(1, 10.0, 5, Side::SELL));
    book.addLimitOrder(Order(2, 20.0, 6, Side::SELL));
    book.addLimitOrder(Order(3, 30.0, 7, Side::SELL));
    REQUIRE(book.getAskBook().begin()->first == 10.0);
}

TEST_CASE("First In First Out Ordering ") {
    OrderBook book;
    book.addLimitOrder(Order(1, 100.0, 5, Side::BUY));
    book.addLimitOrder(Order(2, 100.0, 6, Side::BUY));
    book.addLimitOrder(Order(3, 300.0, 7, Side::BUY));
    REQUIRE(book.getBidBook().at(100.0).front().getId() == 1);
}

TEST_CASE("Full fill") {
    OrderBook book;
    std::ofstream testlog;
    book.addLimitOrder(Order(1, 100.0, 5, Side::BUY));
    book.addLimitOrder(Order(2, 100.0, 5, Side::SELL));

    book.matchOrders(testlog);
    REQUIRE(book.getBidBook().empty());
    REQUIRE(book.getAskBook().empty());
}

TEST_CASE("Partial fill") {
    OrderBook book;
    std::ofstream testlog;
    book.addLimitOrder(Order(1, 100.0, 10, Side::BUY));
    book.addLimitOrder(Order(2, 100.0, 6, Side::SELL));

    book.matchOrders(testlog);
    REQUIRE(book.getBidBook().at(100.0).front().getQuantity() == 4);
    REQUIRE(book.getAskBook().empty());
}

TEST_CASE("Cancel order in bid book") {
    OrderBook book;
    book.addLimitOrder(Order(1, 100.0, 5, Side::BUY));
    book.cancelOrder(1);
    REQUIRE(book.getBidBook().empty());
}

TEST_CASE("Invalid cancel order in bid book passes") {
    OrderBook book;
    book.addLimitOrder(Order(1, 100.0, 5, Side::BUY));
    book.cancelOrder(2);
    REQUIRE(book.getBidBook().count(100.0) == 1);
}

TEST_CASE("Invalid price in order construction throws exception") {
    REQUIRE_THROWS_AS(Order(1, -100.0, 5, Side::BUY), std::invalid_argument);
}

TEST_CASE("Invalid quantity in order construction throws exception") {
    REQUIRE_THROWS_AS(Order(1, 100.0, -1, Side::BUY), std::invalid_argument);
}




