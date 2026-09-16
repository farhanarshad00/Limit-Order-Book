#include "Benchmark.h"
#include "OrderBook.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>

void Benchmark::run(int n){
    OrderBook book;
    std::ofstream devNull("/dev/null");
    std::vector<long long> latencies;
    latencies.reserve(n);

    auto start = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < n; i++){
        double price = 100.0 + (i % 10) * 0.25;
        Side side = (i % 2 == 0) ? Side::BUY : Side::SELL;

        auto opStart = std::chrono::steady_clock::now();

        book.addLimitOrder(Order(i,price,10,side));
        book.matchOrders(devNull);

        auto opEnd = std::chrono::steady_clock::now();
        latencies.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(opEnd - opStart).count());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << duration.count() << " microseconds\n";
    double seconds = duration.count() / 1000000.0;
    double throughput = n / seconds;
    std::cout <<"Total Time in Seconds: " << seconds << std::endl;
    std::cout << "Throughput: " << static_cast<int>(throughput) << " orders/sec\n";

    std::vector<long long> warm(latencies.begin() + 1000, latencies.end());

    std::vector<long long> forP99 = warm;
    size_t n99 = forP99.size() * 99 / 100;
    std::nth_element(forP99.begin(), forP99.begin() + n99, forP99.end());
    long long p99 = forP99[n99];

    std::vector<long long> forP50 = warm;
    size_t n50 = forP50.size() * 50 / 100;
    std::nth_element(forP50.begin(), forP50.begin() + n50, forP50.end());
    long long p50 = forP50[n50];

    std::cout << "p50 latency: " << p50 << " ns\n";
    std::cout << "p99 latency: " << p99 << " ns\n";


}