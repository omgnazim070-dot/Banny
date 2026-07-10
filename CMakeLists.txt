cmake_minimum_required(VERSION 3.20)

project(Banny)

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
    "src/market/Ticker.h" "src/market/Ticker.cpp"
 src/market/Ticker.cpp "src/profit/TradePrices.h" "src/market/IMarketDataProvider.h" "src/market/TestMarketDataProvider.h" "src/market/TestMarketDataProvider.cpp" "src/market/SymbolExtractor.h" "src/market/SymbolExtractor.cpp" "src/paper/PaperTradeJournal.h" "src/paper/PaperTradeJournal.cpp" "src/core/Statistics.h" "src/paper/PaperTradeCsvLogger.h" "src/paper/PaperTradeCsvLogger.cpp" "src/market/MarketSnapshotLogger.h" "src/market/MarketSnapshotLogger.cpp" "src/market/SymbolRegistry.h" "src/market/TestSymbolRegistryProvider.h" "src/market/TestSymbolRegistryProvider.cpp")