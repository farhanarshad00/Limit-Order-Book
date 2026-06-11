#include "Benchmark.h"
#include "OrderBook.h"
#include <fstream>
#include <iostream>
#include <chrono>

void Benchmark::run(int n){
    OrderBook book;
    std::ofstream devNull("/dev/null");

    auto start = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < n; i++){
        double price = 100.0 + (i % 10) * 0.25;
        Side side = (i % 2 == 0) ? Side::BUY : Side::SELL;
        book.addLimitOrder(Order(i,price,10,side));
        book.matchOrders(devNull);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << duration.count() << " microseconds\n";
    double seconds = duration.count() / 1000000.0;
    double throughput = n / seconds;
    std::cout <<"Total Time in Second: " << seconds << std::endl;
    std::cout <<"Total Throughput in Second: " << throughput << std::endl;


}