#include "ArbitrageEngine.h"

#include <iostream>

void ArbitrageEngine::Analyze(
    const std::vector<Triangle>& triangles,
    const MarketData& marketData)
{
    ProfitCalculator calculator;
    SymbolResolver resolver;

    double bestProfit = -999.0;
    Triangle bestTriangle;

    std::cout << std::endl;

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

        if (marketData.prices.find(pairs.pair1) ==
            marketData.prices.end())
        {
            continue;
        }

        if (marketData.prices.find(pairs.pair2) ==
            marketData.prices.end())
        {
            continue;
        }

        if (marketData.prices.find(pairs.pair3) ==
            marketData.prices.end())
        {
            continue;
        }

        double rate1 =
            marketData.prices.at(pairs.pair1);

        double rate2 =
            marketData.prices.at(pairs.pair2);

        double rate3 =
            marketData.prices.at(pairs.pair3);

        auto result =
            calculator.Calculate(
                1000.0,
                rate1,
                rate2,
                rate3);

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
            << "Pairs: "
            << pairs.pair1
            << " | "
            << pairs.pair2
            << " | "
            << pairs.pair3
            << std::endl
            << std::endl;

        if (result.profitPercent > bestProfit)
        {
            bestProfit = result.profitPercent;
            bestTriangle = triangle;
        }
    }

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
}