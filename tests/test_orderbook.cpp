#include <catch2/catch_test_macros.hpp>
#include "OrderBook.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

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

TEST_CASE("Market BUY filling against a resting ASK") {
    OrderBook book;
    std::ofstream testlog;
    book.addLimitOrder(Order(1, 100.0, 5, Side::SELL));
    book.addMarketOrder(Order(2,0,5,Side::BUY),testlog);
    REQUIRE(book.getBidBook().empty());
    REQUIRE(book.getAskBook().empty());
}

TEST_CASE("Market SELL filling against a resting BID") {
    OrderBook book;
    std::ofstream testlog;
    book.addLimitOrder(Order(1, 100.0, 5, Side::BUY));
    book.addMarketOrder(Order(2,0,5,Side::SELL),testlog);
    REQUIRE(book.getBidBook().empty());
    REQUIRE(book.getAskBook().empty());
}

TEST_CASE("Market BUY partially filling against a resting ASK leaving remaining quantity unfilled") {
    OrderBook book;
    std::ofstream testlog;
    book.addLimitOrder(Order(1, 100.0, 5, Side::SELL));
    book.addMarketOrder(Order(2,0,2,Side::BUY),testlog);
    REQUIRE(book.getBidBook().empty());
    REQUIRE(book.getAskBook().at(100.0).front().getQuantity() == 3);
}

TEST_CASE("Modified order inserts into bid book and resets queue position") {
    OrderBook book;
    book.addLimitOrder(Order(1, 100.0, 6, Side::BUY));
    book.addLimitOrder(Order(2, 100.0, 5, Side::BUY));
    book.modifyOrder(1,100.0,10);
    REQUIRE(book.getBidBook().at(100.0).front().getId() == 2);
}

TEST_CASE("Cancel that empties a bid level erases only that level") {
    OrderBook book;
    book.addLimitOrder(Order(1, 100.0, 5, Side::BUY));
    book.addLimitOrder(Order(2, 99.0, 5, Side::BUY));
    book.cancelOrder(1);
    REQUIRE(book.getBidBook().count(100.0) == 0);
    REQUIRE(book.getBidBook().count(99.0) == 1);

    book.addLimitOrder(Order(3, 100.0, 4, Side::BUY));
    REQUIRE(book.getBidBook().begin()->first == 100.0);
    book.cancelOrder(3);
    REQUIRE(book.getBidBook().count(100.0) == 0);
    REQUIRE(book.getBidBook().size() == 1);
}

TEST_CASE("Cancel that empties an ask level erases only that level") {
    OrderBook book;
    book.addLimitOrder(Order(1, 101.0, 5, Side::SELL));
    book.addLimitOrder(Order(2, 102.0, 5, Side::SELL));
    book.cancelOrder(1);
    REQUIRE(book.getAskBook().count(101.0) == 0);
    REQUIRE(book.getAskBook().begin()->first == 102.0);
}

TEST_CASE("Cancel from the middle of a level keeps time priority of the rest") {
    OrderBook book;
    book.addLimitOrder(Order(1, 100.0, 5, Side::BUY));
    book.addLimitOrder(Order(2, 100.0, 5, Side::BUY));
    book.addLimitOrder(Order(3, 100.0, 5, Side::BUY));
    book.cancelOrder(2);
    const auto& level = book.getBidBook().at(100.0);
    REQUIRE(level.size() == 2);
    REQUIRE(level.front().getId() == 1);
    REQUIRE(level.back().getId() == 3);
}

TEST_CASE("Cancel after a partial fill removes the remainder") {
    OrderBook book;
    std::ofstream testlog;
    book.addLimitOrder(Order(1, 100.0, 10, Side::BUY));
    book.addLimitOrder(Order(2, 100.0, 4, Side::SELL));
    book.matchOrders(testlog);
    book.cancelOrder(1);
    REQUIRE(book.getBidBook().empty());
    REQUIRE(book.getAskBook().empty());
}

TEST_CASE("Modify that decreases quantity keeps queue position") {
    OrderBook book;
    book.addLimitOrder(Order(1, 100.0, 6, Side::BUY));
    book.addLimitOrder(Order(2, 100.0, 5, Side::BUY));
    book.modifyOrder(1, 100.0, 3);
    const auto& level = book.getBidBook().at(100.0);
    REQUIRE(level.front().getId() == 1);
    REQUIRE(level.front().getQuantity() == 3);
}

TEST_CASE("Modify that changes price moves the order to the back of the new level") {
    OrderBook book;
    book.addLimitOrder(Order(1, 100.0, 5, Side::BUY));
    book.addLimitOrder(Order(2, 101.0, 5, Side::BUY));
    book.modifyOrder(1, 101.0, 5);
    REQUIRE(book.getBidBook().count(100.0) == 0);
    const auto& level = book.getBidBook().at(101.0);
    REQUIRE(level.front().getId() == 2);
    REQUIRE(level.back().getId() == 1);
}

TEST_CASE("Invalid modify throws and leaves the resting order untouched") {
    OrderBook book;
    book.addLimitOrder(Order(1, 100.0, 5, Side::BUY));
    REQUIRE_THROWS_AS(book.modifyOrder(1, 100.0, 0), std::invalid_argument);
    REQUIRE(book.getBidBook().at(100.0).front().getQuantity() == 5);
}

TEST_CASE("Duplicate order ID is rejected") {
    OrderBook book;
    book.addLimitOrder(Order(1, 100.0, 5, Side::BUY));
    REQUIRE_THROWS_AS(book.addLimitOrder(Order(1, 99.0, 5, Side::BUY)), std::invalid_argument);
    REQUIRE(book.getBidBook().count(99.0) == 0);
}

static std::string firstTradePrice(const std::filesystem::path& path) {
    std::ifstream in(path);
    std::string line;
    std::getline(in, line);  // timestamp,buy_id,sell_id,price,quantity
    std::string field;
    std::stringstream ss(line);
    for (int i = 0; i < 4; ++i) std::getline(ss, field, ',');
    return field;
}

TEST_CASE("Market SELL trades at the resting bid's price") {
    auto path = std::filesystem::temp_directory_path() / "lob_market_sell_test.csv";
    {
        OrderBook book;
        std::ofstream log(path);
        book.addLimitOrder(Order(1, 100.0, 5, Side::BUY));
        book.addMarketOrder(Order(2, 0, 5, Side::SELL), log);
    }
    REQUIRE(firstTradePrice(path) == "100");
    std::filesystem::remove(path);
}

TEST_CASE("Incoming buy trades at the resting ask's price") {
    auto path = std::filesystem::temp_directory_path() / "lob_limit_buy_test.csv";
    {
        OrderBook book;
        std::ofstream log(path);
        book.addLimitOrder(Order(1, 100.0, 5, Side::SELL));
        book.addLimitOrder(Order(2, 101.0, 5, Side::BUY));
        book.matchOrders(log);
    }
    REQUIRE(firstTradePrice(path) == "100");
    std::filesystem::remove(path);
}
