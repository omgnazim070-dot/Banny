#include "ArbitrageEngine.h"

#include <iostream>

#include <algorithm>
#include <chrono>

Statistics ArbitrageEngine::Analyze(
    const std::vector<Triangle>& triangles,
    const MarketData& marketData,
    const TradingSettings& settings)
{
    ProfitCalculator calculator;
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
        const std::string& pair1 =
            triangle.pairAB.symbol;

        const std::string& pair2 =
            triangle.pairBC.symbol;

        const std::string& pair3 =
            triangle.pairCA.symbol;

        auto it1 =
            marketData.tickers.find(pair1);

        if (it1 ==
            marketData.tickers.end())
        {
            stats.missingTickerTriangles++;
            continue;
        }

        auto it2 =
            marketData.tickers.find(pair2);

        if (it2 ==
            marketData.tickers.end())
        {
            stats.missingTickerTriangles++;
            continue;
        }

        auto it3 =
            marketData.tickers.find(pair3);

        if (it3 ==
            marketData.tickers.end())
        {
            stats.missingTickerTriangles++;
            continue;
        }

        const auto& ticker1 =
            it1->second;

        const auto& ticker2 =
            it2->second;

        const auto& ticker3 =
            it3->second;

        auto newest =
            std::max({
                ticker1.lastUpdate,
                ticker2.lastUpdate,
                ticker3.lastUpdate
                });

        auto oldest =
            std::min({
                ticker1.lastUpdate,
                ticker2.lastUpdate,
                ticker3.lastUpdate
                });

        auto age =
            std::chrono::duration_cast<
            std::chrono::milliseconds>(
                newest - oldest);

        if (age.count() > 200)
        {
            stats.staleTriangles++;
            continue;
        }

        stats.validTriangles++;

        TradePrices prices;

        /*
        A -> B
        */
        prices.buy1 =
            (triangle.pairAB.quoteAsset ==
                triangle.assetA);

        /*
        B -> C
        */
        prices.buy2 =
            (triangle.pairBC.quoteAsset ==
                triangle.assetB);

        /*
        C -> A
        */
        prices.buy3 =
            (triangle.pairCA.quoteAsset ==
                triangle.assetC);

        prices.price1 =
            prices.buy1
            ? ticker1.askPrice
            : ticker1.bidPrice;

        prices.price2 =
            prices.buy2
            ? ticker2.askPrice
            : ticker2.bidPrice;

        prices.price3 =
            prices.buy3
            ? ticker3.askPrice
            : ticker3.bidPrice;
        prices.buy3 =
            (triangle.pairCA.quoteAsset ==
                triangle.assetC);

        if (prices.price1 <= 0.0 ||
            prices.price2 <= 0.0 ||
            prices.price3 <= 0.0)
        {
            continue;
        }

        stats.analyzedTriangles++;

        auto result =
            calculator.CalculateRealistic(
                settings.startBalance,
                prices,
                settings.commissionPercent / 100.0,
                settings.slippagePercent / 100.0);

        if (triangle.assetA == "BTC" &&
            triangle.assetB == "USDT" &&
            triangle.assetC == "BRL")
        {
            std::cout << "\n========== DEBUG ==========\n";

            std::cout << "Route: "
                << triangle.assetA << " -> "
                << triangle.assetB << " -> "
                << triangle.assetC << " -> "
                << triangle.assetA << std::endl;

            std::cout << "Pair1: " << pair1
                << " buy=" << std::boolalpha << prices.buy1
                << " price=" << prices.price1 << std::endl;

            std::cout << "Pair2: " << pair2
                << " buy=" << std::boolalpha << prices.buy2
                << " price=" << prices.price2 << std::endl;

            std::cout << "Pair3: " << pair3
                << " buy=" << std::boolalpha << prices.buy3
                << " price=" << prices.price3 << std::endl;

            std::cout << "End: " << result.endAmount << std::endl;
            std::cout << "Profit: " << result.profitPercent << "%\n";
        }

        if (result.profitPercent > bestProfit)
        {
            bestProfit =
                result.profitPercent;

            stats.bestProfitPercent =
                result.profitPercent;

            stats.bestRoute =
                triangle.assetA +
                " -> " +
                triangle.assetB +
                " -> " +
                triangle.assetC +
                " -> " +
                triangle.assetA;

            bestTriangle =
                triangle;

            stats.bestTriangle =
                triangle;

            stats.bestPrices =
                prices;

            stats.bestResult =
                result;
        }

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
            << pair1
            << " | "
            << pair2
            << " | "
            << pair3
            << std::endl;

        std::cout
            << "Trade Prices: "
            << prices.price1
            << " | "
            << prices.price2
            << " | "
            << prices.price3
            << std::endl
            << std::endl;

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