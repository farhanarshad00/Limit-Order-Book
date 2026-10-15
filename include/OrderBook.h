#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <cstddef>
#include <functional>
#include <map>
#include <list>
#include <unordered_map>
#include <fstream>
#include "Order.h"

class OrderBook {
    private:
        using BidBook = std::map<double, std::list<Order>, std::greater<double>>;
        using AskBook = std::map<double, std::list<Order>>;

        BidBook bid_book;
        AskBook ask_book;

        struct OrderLocation {
            Side side;
            std::list<Order>::iterator it;
            BidBook::iterator bid_it{};
            AskBook::iterator ask_it{};
        };
        std::unordered_map<int, OrderLocation> order_lookup;


        Side last_added_side = Side::BUY;

        void logTrade(std::ofstream& tradeLog, int buyId, int sellId, double price, int quantity);

    public:
        const BidBook& getBidBook() const { return bid_book; }
        const AskBook& getAskBook() const { return ask_book; }
        void reserve(std::size_t expectedOrders) { order_lookup.reserve(expectedOrders); }
        void addLimitOrder(const Order& order);
        void addMarketOrder(const Order& order, std::ofstream& tradeLog);
        void print() const;
        void matchOrders(std::ofstream& tradeLog);
        void cancelOrder(int id);
        void modifyOrder(int id, double price, int quantity);
};

#endif
