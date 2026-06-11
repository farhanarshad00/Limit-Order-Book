#ifndef REPLAY_H
#define REPLAY_H
#include "OrderBook.h"
#include <fstream>

class Replay {
    public:
        static void run(OrderBook& book,std::string filename,std::ofstream& tradelog);


};

#endif