#include "Order.h"

Order::Order(int id, double price, int quantity, Side side) {
    if (price < 0.0) {
        throw std::invalid_argument("Price cannot be less than 0");
    }
    /*
    if (side != "Buy" && side != "Sell") {
        throw std::invalid_argument("Side must be Buy or Sell");
    }
    */
    this->id = id;
    this->price = price;
    this->quantity = quantity;
    this->side = side;
}

static std::string sideToString(Side side) {
    if (side == Side::BUY) return "BUY";
    if (side == Side::SELL) return "SELL";
    return "UNKNOWN";
}


void Order::print() const{
    std::cout << "ID: " << id << std::endl;
    std::cout << "Quantity: " << quantity << std::endl;
    std::cout << "Side: " << sideToString(side) << std::endl;
}