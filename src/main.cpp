#include <iostream>
#include <vector>

#include "config/Config.h"
#include "logger/Logger.h"
#include "core/bot.h"
#include "scanner/Scanner.h"
#include "triangle/TriangleBuilder.h"
#include "arbitrage/ArbitrageEngine.h"
#include "market/SymbolResolver.h"
#include "binance/BinanceRestClient.h"

int main()
{
    std::cout << "[Banny]" << std::endl;

    Logger logger;

    if (!logger.Start())
    {
        return 1;
    }

    logger.Info("Banny started");

    Config config;
    config.Load();

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

    Bot bot;
    bot.Run();

    TriangleBuilder builder;

    std::vector<std::string> symbols =
    {
        "BTCUSDT",
        "ETHUSDT",
        "ETHBTC",
        "BNBUSDT",
        "BNBBTC"
    };

    auto triangles = builder.Build(symbols);

    std::cout << std::endl;

    for (const auto& t : triangles)
    {
        std::cout
            << t.assetA
            << " -> "
            << t.assetB
            << " -> "
            << t.assetC
            << " -> "
            << t.assetA
            << std::endl;
    }

    std::cout << std::endl;

    SymbolResolver resolver;

    for (const auto& t : triangles)
    {
        auto pairs = resolver.Resolve(
            t.assetA,
            t.assetB,
            t.assetC);

        std::cout
            << pairs.pair1
            << " | "
            << pairs.pair2
            << " | "
            << pairs.pair3
            << std::endl;
    }

    BinanceRestClient binance;

    auto marketData =
        binance.DownloadPrices();

    Scanner scanner;

    ArbitrageEngine engine;

    engine.Analyze(
        triangles,
        marketData);

    scanner.Scan();

    logger.Info("Core initialized");

    return 0;
}