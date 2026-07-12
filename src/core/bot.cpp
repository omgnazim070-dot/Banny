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

    BinanceMarketDataProvider provider;

    auto marketData =
        provider.GetMarketData();

    MarketSnapshotLogger snapshotLogger;

    snapshotLogger.Save(
        marketData);

    BinanceSymbolRegistryProvider symbolProvider;

    auto registry =
        symbolProvider.GetSymbols();

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

    for (const auto& pair : registry.pairs)
    {
        symbols.push_back(
            pair.symbol);
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

    std::cout << std::endl;

    std::cout << std::endl;

    SymbolResolver resolver;

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

    ArbitrageEngine engine;

    auto stats =
        engine.Analyze(
            triangles,
            marketData,
            settings);

    std::cout
        << std::endl
        << "=== BANNY STATISTICS ==="
        << std::endl;

    std::cout
        << "Total Triangles: "
        << stats.totalTriangles
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
        << std::endl
        << std::endl;

    if (stats.profitableTriangles > 0)
    {
        PaperTrade trade;

        trade.route =
            "BEST_ROUTE";

        trade.startBalance =
            settings.startBalance;

        trade.endBalance =
            settings.startBalance *
            (1.0 +
                stats.bestProfitPercent / 100.0);

        trade.profitPercent =
            stats.bestProfitPercent;

        trade.profitUsdt =
            trade.endBalance -
            trade.startBalance;

        journal.Add(trade);
        csvLogger.Append(trade);
    }

    std::cout
        << "Paper Trades: "
        << journal.GetTrades().size()
        << std::endl;

    std::cout
        << std::endl
        << "=== PAPER TRADE HISTORY ==="
        << std::endl;

    int tradeIndex = 1;

    for (const auto& trade :
        journal.GetTrades())
    {
        std::cout
            << "Trade #"
            << tradeIndex++
            << std::endl;

        std::cout
            << "Route: "
            << trade.route
            << std::endl;

        std::cout
            << "Start Balance: "
            << trade.startBalance
            << " USDT"
            << std::endl;

        std::cout
            << "End Balance: "
            << trade.endBalance
            << " USDT"
            << std::endl;

        std::cout
            << "Profit: "
            << trade.profitUsdt
            << " USDT"
            << std::endl;

        std::cout
            << "Profit Percent: "
            << trade.profitPercent
            << "%"
            << std::endl;

        std::cout
            << std::endl;
    }

    Scanner scanner;

    scanner.Scan();

    logger.Info("Core initialized");
}