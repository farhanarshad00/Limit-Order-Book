#include "Replay.h"
#include "OrderBook.h"
#include <fstream>
#include <sstream>

void Replay::run(OrderBook& book,std::string filename,std::ofstream& tradelog){
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Could not open file\n";
        return;
    }
    std::string header;
    std::getline(file, header); 
    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string id_str, type_str, side_str, price_str, qty_str;
        std::getline(ss, id_str, ',');  
        std::getline(ss, type_str, ',');  
        std::getline(ss, side_str, ',');  
        std::getline(ss, price_str, ',');  
        std::getline(ss, qty_str, ',');  

        int id = std::stoi(id_str);    
        double price = std::stod(price_str);  
        int qty = std::stoi(qty_str);  

        Side side = (side_str == "BUY") ? Side::BUY : Side::SELL;

        if (type_str == "limit"){
            book.addLimitOrder(Order(id,price,qty,side));
            book.matchOrders(tradelog);
        } else if (type_str == "market"){
            book.addMarketOrder(Order(id,price,qty,side),tradelog);
        } else if (type_str == "cancel"){
            book.cancelOrder(id);
        } else if (type_str == "modify"){
            book.modifyOrder(id,price,qty);
        }
    }






}