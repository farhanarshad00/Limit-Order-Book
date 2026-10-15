#include "OrderBook.h"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>


void OrderBook::addLimitOrder(const Order& order) {
    auto [lookup_it, inserted] = order_lookup.try_emplace(order.getId());
    if (!inserted) {
        throw std::invalid_argument("Duplicate order ID");
    }
    last_added_side = order.getSide();

    if (order.getSide() == Side::BUY) {
        auto level_it = bid_book.try_emplace(order.getPrice()).first;
        level_it->second.push_back(order);
        lookup_it->second =
            OrderLocation{Side::BUY, std::prev(level_it->second.end()), level_it, {}};
    } else {
        auto level_it = ask_book.try_emplace(order.getPrice()).first;
        level_it->second.push_back(order);
        lookup_it->second =
            OrderLocation{Side::SELL, std::prev(level_it->second.end()), {}, level_it};
    }
}

void OrderBook::addMarketOrder(const Order& order, std::ofstream& tradelog){
    auto m_id = order.getId();
    auto m_quantity = order.getQuantity();
    auto m_side = order.getSide();
    if(m_side==Side::BUY){
        auto m_price = std::numeric_limits<double>::max();
        Order marketOrder(m_id,m_price,m_quantity,m_side);
        addLimitOrder(marketOrder);
        matchOrders(tradelog);
        cancelOrder(m_id);
    }else if(m_side == Side::SELL){
        auto m_price = 0.0;
        Order marketOrder(m_id,m_price,m_quantity,m_side);
        addLimitOrder(marketOrder);
        matchOrders(tradelog);
        cancelOrder(m_id);
    }
}

void OrderBook::print() const{
    std::cout << "--- ASK BOOK ---" << std::endl;
    for (const auto& [price, orderList] : ask_book) {
        for (const auto& o : orderList) {
            std::cout << "Price: " << price << std::endl;
            o.print();
            std::cout << std::endl;
        }
    }

    std::cout << "--- BID BOOK ---" << std::endl;
    for (const auto& [price, orderList] : bid_book) {
        for (const auto& o : orderList) {
            std::cout << "Price: " << price << std::endl;
            o.print();
            std::cout << std::endl;
        }
    }
}

void OrderBook::matchOrders(std::ofstream& tradeLog){

    while (!bid_book.empty() && !ask_book.empty()){
        if (bid_book.begin()->first >= ask_book.begin()->first){
            Order& bestBid = bid_book.begin()->second.front();
            Order& bestAsk = ask_book.begin()->second.front();
            int tradedQty = std::min(bestBid.getQuantity(), bestAsk.getQuantity());
            bestBid.setQuantity(bestBid.getQuantity() - tradedQty);
            bestAsk.setQuantity(bestAsk.getQuantity() - tradedQty);


            double tradePrice = (last_added_side == Side::BUY)
                ? ask_book.begin()->first
                : bid_book.begin()->first;
            logTrade(tradeLog, bestBid.getId(), bestAsk.getId(), tradePrice, tradedQty);

            if(bestBid.getQuantity() == 0){
                int bidId = bestBid.getId();
                bid_book.begin()->second.erase(bid_book.begin()->second.begin());
                if (bid_book.begin()->second.empty()) {
                    bid_book.erase(bid_book.begin());
                }
                order_lookup.erase(bidId);
            }
            if(bestAsk.getQuantity() == 0){
                int askId = bestAsk.getId();
                ask_book.begin()->second.erase(ask_book.begin()->second.begin());
                if (ask_book.begin()->second.empty()) {
                    ask_book.erase(ask_book.begin());
                }
                order_lookup.erase(askId);
            }
        }else{
            break;
        }
    }
}

void OrderBook::logTrade(std::ofstream& tradeLog, int buyId, int sellId, double price, int quantity){
    auto now = std::chrono::system_clock::now();
    time_t t = std::chrono::system_clock::to_time_t(now);
    std::string timestamp = ctime(&t);
    timestamp.pop_back();  // remove newline
    tradeLog << timestamp << "," << buyId << ","
             << sellId << "," << price
             << "," << quantity << "\n";
}


void OrderBook::cancelOrder(int id){
    auto it = order_lookup.find(id);
    if (it == order_lookup.end()) return;

    const OrderLocation& loc = it->second;
    if (loc.side == Side::BUY) {
        auto level_it = loc.bid_it;
        level_it->second.erase(loc.it);
        if (level_it->second.empty()) bid_book.erase(level_it);
    } else {
        auto level_it = loc.ask_it;
        level_it->second.erase(loc.it);
        if (level_it->second.empty()) ask_book.erase(level_it);
    }
    order_lookup.erase(it);
}


void OrderBook::modifyOrder(int id, double price, int quantity){
    auto it = order_lookup.find(id);
    if (it == order_lookup.end()) return;

    Side side = it->second.side;
    Order replacement(id, price, quantity, side);

    Order& resting = *it->second.it;
    if (price == resting.getPrice() && quantity <= resting.getQuantity()) {
        resting.setQuantity(quantity);
        return;
    }
    cancelOrder(id);
    addLimitOrder(replacement);
}
