cmake_minimum_required(VERSION 3.20)

project(Banny)

find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Banny

    src/main.cpp

    src/config/Config.cpp
    src/logger/Logger.cpp
    src/core/bot.cpp

    src/scanner/Scanner.cpp

    src/triangle/TriangleBuilder.cpp

    src/profit/ProfitCalculator.cpp

    src/arbitrage/ArbitrageEngine.cpp

    src/market/SymbolResolver.cpp

    src/binance/BinanceRestClient.cpp
)

target_link_libraries(
    Banny
    OpenSSL::SSL
    OpenSSL::Crypto
)