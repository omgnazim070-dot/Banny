#include "ArbitrageEngine.h"

#include <iostream>

Statistics ArbitrageEngine::Analyze(
    const std::vector<Triangle>& triangles,
    const MarketData& marketData,
    const TradingSettings& settings)
{
    ProfitCalculator calculator;
    SymbolResolver resolver;
    Statistics stats;

    double bestProfit = -999.0;
    Triangle bestTriangle;

    int profitableCount = 0;
    int rejectedCount = 0;

    std::cout << std::endl;

    stats.totalTriangles =
        static_cast<int>(
            triangles.size());

    std::cout
        << "Found "
        << triangles.size()
        << " triangles"
        << std::endl
        << std::endl;

    for (const auto& triangle : triangles)
    {
        auto pairs = resolver.Resolve(
            triangle.assetA,
            triangle.assetB,
            triangle.assetC);

        if (marketData.tickers.find(pairs.pair1) ==
            marketData.tickers.end())
        {
            continue;
        }

        if (marketData.tickers.find(pairs.pair2) ==
            marketData.tickers.end())
        {
            continue;
        }

        if (marketData.tickers.find(pairs.pair3) ==
            marketData.tickers.end())
        {
            continue;
        }

        const auto& ticker1 =
            marketData.tickers.at(
                pairs.pair1);

        const auto& ticker2 =
            marketData.tickers.at(
                pairs.pair2);

        const auto& ticker3 =
            marketData.tickers.at(
                pairs.pair3);

        TradePrices prices;

        prices.buyPrice1 =
            ticker1.askPrice;

        prices.buyPrice2 =
            ticker2.askPrice;

        prices.sellPrice3 =
            ticker3.bidPrice;

        auto result =
            calculator.CalculateRealistic(
                settings.startBalance,
                prices,
                settings.commissionPercent / 100.0,
                settings.slippagePercent / 100.0);

        if (result.profitPercent <
            settings.minProfitPercent)
        {
            rejectedCount++;
            stats.rejectedTriangles++;
            continue;
        }

        profitableCount++;
        stats.profitableTriangles++;

        std::cout
            << triangle.assetA
            << " -> "
            << triangle.assetB
            << " -> "
            << triangle.assetC
            << " -> "
            << triangle.assetA
            << std::endl;

        std::cout
            << "Profit: "
            << result.profitPercent
            << "%"
            << std::endl;

        std::cout
            << "Start Balance: "
            << result.startAmount
            << " USDT"
            << std::endl;

        std::cout
            << "End Balance: "
            << result.endAmount
            << " USDT"
            << std::endl;

        std::cout
            << "Net Profit: "
            << (result.endAmount -
                result.startAmount)
            << " USDT"
            << std::endl;

        std::cout
            << "Pairs: "
            << pairs.pair1
            << " | "
            << pairs.pair2
            << " | "
            << pairs.pair3
            << std::endl;

        std::cout
            << "Trade Prices: "
            << prices.buyPrice1
            << " | "
            << prices.buyPrice2
            << " | "
            << prices.sellPrice3
            << std::endl
            << std::endl;

        if (result.profitPercent > bestProfit)
        {
            bestProfit =
                result.profitPercent;

            stats.bestProfitPercent =
                result.profitPercent;

            bestTriangle =
                triangle;
        }
    }

    std::cout
        << "Profitable triangles: "
        << profitableCount
        << std::endl;

    std::cout
        << "Rejected triangles: "
        << rejectedCount
        << std::endl
        << std::endl;

    if (profitableCount > 0)
    {
        std::cout
            << "Best Opportunity"
            << std::endl;

        std::cout
            << bestTriangle.assetA
            << " -> "
            << bestTriangle.assetB
            << " -> "
            << bestTriangle.assetC
            << " -> "
            << bestTriangle.assetA
            << std::endl;

        std::cout
            << "Profit: "
            << bestProfit
            << "%"
            << std::endl;

        std::cout
            << "Configured Balance: "
            << settings.startBalance
            << " USDT"
            << std::endl;
    }
    else
    {
        std::cout
            << "No profitable opportunities found"
            << std::endl;
    }

    return stats;
}