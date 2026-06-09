#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <map>
#include <list>
#include <unordered_map>
#include <fstream>
#include "Order.h"

class OrderBook {
    private:
        std::map<double, std::list<Order>, std::greater<double>> bid_book;
        std::map<double, std::list<Order>> ask_book;
        struct OrderLocation {
            double price;
            Side side;
            std::list<Order>::iterator it;
        };
        std::unordered_map<int, OrderLocation> order_lookup;

        

        void logTrade(std::ofstream& tradeLog, int buyId, int sellId, double price, int quantity);

    public:
        void addLimitOrder(const Order& order);
        void addMarketOrder(const Order& order, std::ofstream& tradeLog);
        void print() const;
        void matchOrders(std::ofstream& tradeLog);
        void cancelOrder(int id);
        void modifyOrder(int id,double price, int quantity);
    };

#endif