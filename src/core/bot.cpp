#include "Bot.h"

#include <iostream>
#include <vector>

#include "../config/Config.h"
#include "../config/TradingSettings.h"

#include "../logger/Logger.h"

#include "../scanner/Scanner.h"
#include "../triangle/TriangleBuilder.h"
#include "../arbitrage/ArbitrageEngine.h"

#include "../market/SymbolResolver.h"
#include "../market/TestSymbolRegistryProvider.h"

#include "../market/BinanceMarketDataProvider.h"
#include "../paper/PaperTradeJournal.h"

#include "../paper/PaperTradeCsvLogger.h"
#include "../market/MarketSnapshotLogger.h"

#include "../market/BinanceSymbolRegistryProvider.h"

#include <thread>
#include <chrono>

#include "../market/MarketMonitor.h"
#include "../binance/BinanceWebSocketClient.h"

#include "../market/MarketDataCache.h"
#include "../../third_party/json/json.hpp"

#include <iomanip>
#include "../market/WebSocketMarketDataProvider.h"

#include "../index/SymbolRegistryIndex.h"
#include "../index/IndexedTriangleBuilder.h"
#include "../index/TriangleIndex.h"
#include "../index/DirtyQueue.h"
#include "../index/IndexedMarketCache.h"
#include "../index/RealtimeIndexer.h"

#include <cmath>

using json = nlohmann::json;

void Bot::Run()
{
    std::cout << "[Banny]" << std::endl;

    Logger logger;

    if (!logger.Start())
    {
        return;
    }

    logger.Info("Banny started");

    Config config;

    if (!config.Load())
    {
        return;
    }

    std::cout
        << "Bot Name: "
        << config.BotName
        << std::endl;

    std::cout
        << "Trading Mode: "
        << config.TradingMode
        << std::endl;

    std::cout
        << "Scan Interval: "
        << config.ScanIntervalMs
        << std::endl;

    std::cout
        << "Min Profit: "
        << config.MinProfitPercent
        << std::endl;

    logger.Info("Config loaded");

    BinanceSymbolRegistryProvider symbolProvider;

    auto registry =
        symbolProvider.GetSymbols();

    std::vector<std::string> wsSymbols;


    for (const auto& pair : registry.pairs)
    {
        wsSymbols.push_back(
            pair.symbol);
    }

    std::cout
        << "\n===== BOT SYMBOL CHECK ====="
        << std::endl;

    for (const auto& s : wsSymbols)
    {
        if (s.find_first_not_of(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != std::string::npos)
        {
            std::cout
                << "BOT BAD SYMBOL: "
                << s
                << std::endl;
        }
    }

    MarketSnapshotLogger snapshotLogger;

    std::cout
        << "Pairs loaded: "
        << registry.pairs.size()
        << std::endl;

    if (!registry.pairs.empty())
    {
        const auto& pair =
            registry.pairs.front();

        std::cout
            << pair.symbol
            << " | "
            << pair.baseAsset
            << " | "
            << pair.quoteAsset
            << std::endl;
    }

    std::vector<std::string> symbols;

    std::cout
        << "Registry pairs: "
        << registry.pairs.size()
        << std::endl;

    for (size_t i = 0;
        i < std::min<size_t>(20, registry.pairs.size());
        ++i)
    {
        std::cout
            << registry.pairs[i].symbol
            << std::endl;;
    }

    std::cout
        << "STEP 4"
        << std::endl;

    TriangleBuilder builder;

    auto triangles =
        builder.Build(
            registry.pairs);

    SymbolRegistryIndex symbolIndex;

    IndexedTriangleBuilder indexedBuilder;

    auto indexedTriangles =
        indexedBuilder.Build(
            triangles,
            symbolIndex);

    std::cout
        << "Indexed symbols after build: "
        << symbolIndex.Size()
        << std::endl;

    TriangleIndex triangleIndex;

    triangleIndex.Build(
        indexedTriangles,
        symbolIndex.Size());

    IndexedMarketCache indexedCache;

    indexedCache.Resize(
        symbolIndex.Size());

    DirtyQueue dirtyQueue;

    RealtimeIndexer realtimeIndexer(
        symbolIndex,
        triangleIndex,
        dirtyQueue,
        indexedCache);

    WebSocketMarketDataProvider wsProvider;

    wsProvider.SetRealtimeIndexer(
        &realtimeIndexer);

    if (!wsProvider.Start(
        wsSymbols))
    {
        std::cout
            << "STEP 1"
            << std::endl;

        std::cout
            << "WebSocket provider start failed"
            << std::endl;

        return;
    }

    std::cout
        << "STEP 2"
        << std::endl;

    std::this_thread::sleep_for(
        std::chrono::seconds(3));

    std::cout
        << "STEP 3"
        << std::endl;

    std::cout
        << "Initial market warmup completed"
        << std::endl;

    for (SymbolId symbol = 0;
        symbol < symbolIndex.Size();
        ++symbol)
    {
        const auto& list =
            triangleIndex.GetTriangles(
                symbol);

        for (TriangleId triangle : list)
        {
            dirtyQueue.Mark(
                triangle);
        }
    }

    std::cout
        << "Dirty queue initialized"
        << std::endl;

    std::cout
        << "Indexed symbols: "
        << symbolIndex.Size()
        << std::endl;

    std::cout
        << "Indexed triangles: "
        << indexedTriangles.size()
        << std::endl;

    std::cout
        << std::endl
        << "Found "
        << triangles.size()
        << " triangles"
        << std::endl
        << std::endl;

    TradingSettings settings;

    settings.minProfitPercent =
        config.MinProfitPercent;

    settings.commissionPercent =
        config.CommissionPercent;

    settings.slippagePercent =
        config.SlippagePercent;

    settings.startBalance =
        config.StartBalance;

    PaperTradeJournal journal;

    PaperTradeCsvLogger csvLogger;

    MarketMonitor monitor;

    double lastBestProfit = -999.0;

    while (true)
    {


        auto marketData =
            wsProvider.GetSnapshot();

        snapshotLogger.Save(
            marketData);

        auto analysisStart =
            std::chrono::high_resolution_clock::now();

        auto stats =
            monitor.RunDirty(
                indexedTriangles,
                dirtyQueue,
                indexedCache,
                symbolIndex,
                settings);

        auto analysisEnd =
            std::chrono::high_resolution_clock::now();

        auto analysisTime =
            std::chrono::duration_cast<
            std::chrono::milliseconds>(
                analysisEnd - analysisStart);


        std::cout
            << "Analysis time: "
            << analysisTime.count()
            << " ms"
            << std::endl;


        if (analysisTime.count() > 0)
        {
            std::cout
                << "Triangles/sec: "
                <<
                (stats.analyzedTriangles * 1000)
                /
                analysisTime.count()
                << std::endl;
        }

        if (std::abs(
            stats.bestProfitPercent - lastBestProfit)
        > 0.000001)
        {
            std::cout
                << "\n===== NEW BEST ====="
                << std::endl;

            std::cout
                << "Route: "
                << stats.bestRoute
                << std::endl;

            std::cout
                << "Profit: "
                << stats.bestProfitPercent
                << "%"
                << std::endl;

            std::cout
                << "End Amount: "
                << stats.bestResult.endAmount
                << std::endl;

            lastBestProfit =
                stats.bestProfitPercent;
        }

        std::cout
            << std::endl
            << "=== BANNY STATISTICS ==="
            << std::endl;

        std::cout
            << "WebSocket cache symbols: "
            << marketData.tickers.size()
            << std::endl;

        std::cout
            << "Total Triangles: "
            << stats.totalTriangles
            << std::endl;

        std::cout
            << "Analyzed: "
            << stats.analyzedTriangles
            << std::endl;

        std::cout
            << "Valid Triangles: "
            << stats.validTriangles
            << std::endl;

        std::cout
            << "Stale Triangles: "
            << stats.staleTriangles
            << std::endl;

        std::cout
            << "Profitable: "
            << stats.profitableTriangles
            << std::endl;

        std::cout
            << "Rejected: "
            << stats.rejectedTriangles
            << std::endl;

        std::cout
            << "Best Profit: "
            << stats.bestProfitPercent
            << "%"
            << std::endl;

        std::cout
            << "Best Route: "
            << stats.bestRoute
            << std::endl;

        /*
        std::cout
            << "\n===== BEST TRIANGLE ====="
            << std::endl;

        std::cout
            << "Pair 1: "
            << stats.bestTriangle.pairAB.symbol
            << std::endl;

        std::cout
            << "Pair 2: "
            << stats.bestTriangle.pairBC.symbol
            << std::endl;

        std::cout
            << "Pair 3: "
            << stats.bestTriangle.pairCA.symbol
            << std::endl;

        std::cout
            << "Price 1: "
            << stats.bestPrices.price1
            << std::endl;

        std::cout
            << "Buy 1: "
            << std::boolalpha
            << stats.bestPrices.buy1
            << std::endl;

        std::cout
            << "Buy 2: "
            << std::boolalpha
            << stats.bestPrices.buy2
            << std::endl;

        std::cout
            << "Buy 3: "
            << std::boolalpha
            << stats.bestPrices.buy3
            << std::endl;

        std::cout
            << "Price 2: "
            << stats.bestPrices.price2
            << std::endl;

        std::cout
            << "Price 3: "
            << stats.bestPrices.price3
            << std::endl;

        std::cout
            << "Start Amount: "
            << stats.bestResult.startAmount
            << std::endl;

        std::cout
            << "End Amount: "
            << stats.bestResult.endAmount
            << std::endl;
        */

        std::this_thread::sleep_for(
            std::chrono::milliseconds(
                config.ScanIntervalMs));
    }

    logger.Info("Core initialized");

}