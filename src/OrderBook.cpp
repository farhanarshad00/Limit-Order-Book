#include "OrderBook.h"
#include <chrono>
#include <ctime>
#include <string>


void OrderBook::addLimitOrder(const Order& order) {
    if (order.getSide() == Side::BUY) {
        auto& level = bid_book[order.getPrice()];
        level.push_back(order);
        order_lookup[order.getId()] =
            OrderLocation{order.getPrice(), Side::BUY, std::prev(level.end())};
    } else {
        auto& level = ask_book[order.getPrice()];
        level.push_back(order);
        order_lookup[order.getId()] =
            OrderLocation{order.getPrice(), Side::SELL, std::prev(level.end())};
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
        
        //std::cout << "FILL: " << tradedQty << " shares at $" << ask_book.begin()->first << std::endl;
        
        logTrade(tradeLog, bestBid.getId(), bestAsk.getId(), ask_book.begin()->first, tradedQty);
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


        }
            else{
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
    if (it != order_lookup.end()){
        auto cancel_level = it->second.price;
        auto cancel_side = it->second.side;
        auto cancel_iter = it->second.it;

        
        if(cancel_side == Side::BUY){
            bid_book[cancel_level].erase(cancel_iter);
            if(bid_book[cancel_level].empty()){
                bid_book.erase(cancel_level);
            }
        }else if(cancel_side == Side::SELL){
            ask_book[cancel_level].erase(cancel_iter);
            if(ask_book[cancel_level].empty()){
                ask_book.erase(cancel_level);
            }
        }

        order_lookup.erase(id);
    
    
    }
}

void OrderBook::modifyOrder(int id, double price, int quantity){
    auto iter = order_lookup.find(id);
    if(iter != order_lookup.end()){
        auto newo_side = iter->second.side;
        cancelOrder(id);
        addLimitOrder(Order(id, price, quantity, newo_side));
        
    }
    
    
}
