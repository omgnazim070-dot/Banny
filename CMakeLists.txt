cmake_minimum_required(VERSION 3.20)

project(Banny)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Banny

    src/main.cpp

    src/config/Config.cpp
    src/logger/Logger.cpp
    src/core/bot.cpp
    src/core/LatencyHistogram.h
    src/core/WorkerIntervalStats.h

    src/scanner/Scanner.cpp

    src/triangle/TriangleBuilder.cpp

    src/profit/ProfitCalculator.cpp

    src/arbitrage/ArbitrageEngine.cpp

    src/market/SymbolResolver.cpp

    src/index/SymbolRegistryIndex.cpp
    src/index/TriangleIndex.cpp
    src/index/IndexedTriangleBuilder.cpp
    src/index/DirtyQueue.cpp
    src/index/IndexedMarketCache.cpp
    src/index/RealtimeIndexer.cpp
    src/index/TriangleRuntimeState.h

    src/binance/BinanceRestClient.cpp
    "src/market/Ticker.h" "src/market/Ticker.cpp"
    src/market/Ticker.cpp "src/profit/TradePrices.h" "src/market/IMarketDataProvider.h" "src/market/TestMarketDataProvider.h" "src/market/TestMarketDataProvider.cpp" "src/market/SymbolExtractor.h" "src/market/SymbolExtractor.cpp" "src/paper/PaperTradeJournal.h" "src/paper/PaperTradeJournal.cpp" "src/core/Statistics.h" "src/paper/PaperTradeCsvLogger.h" "src/paper/PaperTradeCsvLogger.cpp" "src/market/MarketSnapshotLogger.h" "src/market/MarketSnapshotLogger.cpp" "src/market/SymbolRegistry.h" "src/market/TestSymbolRegistryProvider.h" "src/market/TestSymbolRegistryProvider.cpp" "src/market/ISymbolRegistryProvider.h" "src/binance/BinanceExchangeInfoClient.h" "src/binance/BinanceExchangeInfoClient.cpp" "src/market/BinanceSymbolRegistryProvider.h" "src/market/BinanceSymbolRegistryProvider.cpp"
    src/binance/BinanceExchangeInfoClient.cpp
    src/market/BinanceSymbolRegistryProvider.cpp "src/market/BinanceMarketDataProvider.h"
    src/market/BinanceMarketDataProvider.cpp "src/market/MarketMonitor.h" "src/market/MarketMonitor.cpp"
    src/market/MarketMonitor.cpp "src/binance/BinanceWebSocketClient.cpp"
    src/binance/BinanceWebSocketClient.cpp "src/market/MarketDataCache.cpp"
    src/market/MarketDataCache.h
    src/market/MarketDataCache.cpp
    src/market/WebSocketMarketDataProvider.h
    src/market/WebSocketIntervalStats.h
    src/market/WebSocketMarketDataProvider.cpp
 "src/index/SymbolId.h" "src/index/SymbolRegistryIndex.h" "src/index/SymbolRegistryIndex.cpp" "src/index/TriangleIndex.h" "src/index/TriangleIndex.cpp" "src/index/IndexedTriangleBuilder.h" "src/index/IndexedTriangleBuilder.cpp" "src/index/DirtyQueue.h" "src/index/DirtyQueue.cpp" "src/index/IndexedTicker.h" "src/index/IndexedMarketData.h" "src/index/IndexedMarketCache.h" "src/index/IndexedMarketCache.cpp" "src/index/RealtimeIndexer.h" "src/index/RealtimeIndexer.cpp")

    target_link_libraries(
    Banny
    winhttp
    )