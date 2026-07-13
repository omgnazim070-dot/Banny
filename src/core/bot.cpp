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

    std::cout
        << "DEBUG A"
        << std::endl;

    BinanceSymbolRegistryProvider symbolProvider;

    std::cout
        << "DEBUG B1"
        << std::endl;

    auto registry =
        symbolProvider.GetSymbols();

    std::cout
        << "DEBUG B2"
        << std::endl;

    std::vector<std::string> wsSymbols;

    std::cout
        << "DEBUG C"
        << std::endl;

    for (const auto& pair : registry.pairs)
    {
        wsSymbols.push_back(
            pair.symbol);
    }

    WebSocketMarketDataProvider wsProvider;

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

    for (size_t i = 0;
        i < std::min<size_t>(1200, registry.pairs.size());
        ++i)
    {
        wsSymbols.push_back(
            registry.pairs[i].symbol);
    }

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

    while (true)
    {


        auto marketData =
            wsProvider.GetSnapshot();

        snapshotLogger.Save(
            marketData);

        auto stats =
            monitor.Run(
                triangles,
                marketData,
                settings);

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
            << "Missing Tickers: "
            << stats.missingTickerTriangles
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

        std::this_thread::sleep_for(
            std::chrono::milliseconds(
                config.ScanIntervalMs));
    }

    logger.Info("Core initialized");

}