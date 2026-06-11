cmake_minimum_required(VERSION 3.20)
project(OrderBook CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(orderbook 
    src/main.cpp
    src/Order.cpp
    src/OrderBook.cpp
    src/Replay.cpp
)

# Download Catch2 automatically
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)

# Test executable
add_executable(tests
    tests/test_orderbook.cpp
    src/Order.cpp
    src/OrderBook.cpp
    src/Replay.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_include_directories(tests PRIVATE include)