cmake_minimum_required(VERSION 3.20)
project(OrderBook CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(orderbook 
    src/main.cpp
    src/Order.cpp
    src/OrderBook.cpp
)