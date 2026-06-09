#ifndef ORDER_H
#define ORDER_H

#include <iostream>
#include <string>
#include <stdexcept>



enum class Side {BUY,SELL};

class Order {
    private:
        int id;
        double price;
        int quantity;
        Side side;
        

    public:
        Order(int id, double price, int quantity, Side side);
        int getId() const { return id; }
        double getPrice() const { return price; }
        int getQuantity() const { return quantity; }
        Side getSide() const { return side; }
        void print() const;
        void setQuantity(int q) { quantity = q; }
};

#endif