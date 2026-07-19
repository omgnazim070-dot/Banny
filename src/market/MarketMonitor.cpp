#include "MarketMonitor.h"

#include "../arbitrage/ArbitrageEngine.h"

#include "MarketMonitor.h"
#include "../profit/ProfitCalculator.h"
#include "../index/SymbolRegistryIndex.h"
#include "../profit/TradePrices.h"
#include "../profit/ProfitCalculator.h"
#include <iostream>

Statistics MarketMonitor::Run(
    const std::vector<Triangle>& triangles,
    const MarketData& marketData,
    const TradingSettings& settings)

{
    ArbitrageEngine engine;

    return engine.Analyze(
        triangles,
        marketData,
        settings);
}

Statistics MarketMonitor::RunDirty(
    const std::vector<IndexedTriangle>& indexedTriangles,
    DirtyQueue& dirtyQueue,
    IndexedMarketCache& marketCache,
    SymbolRegistryIndex& symbolIndex,
    const TradingSettings& settings)
{
    Statistics stats;

    int dirtyProcessed = 0;

    ProfitCalculator calculator;

    auto market =
        marketCache.Snapshot();


    stats.totalTriangles =
        static_cast<int>(
            indexedTriangles.size());


    while (!dirtyQueue.Empty())
    {
        TriangleId id =
            dirtyQueue.Pop();

        dirtyQueue.Complete(id);

        dirtyProcessed++;


        if (id >= indexedTriangles.size())
        {
            continue;
        }


        const auto& triangle =
            indexedTriangles[id];


        const auto& ticker1 =
            market.Get(
                triangle.pairAB);


        const auto& ticker2 =
            market.Get(
                triangle.pairBC);


        const auto& ticker3 =
            market.Get(
                triangle.pairCA);


        if (!ticker1.valid ||
            !ticker2.valid ||
            !ticker3.valid)
        {
            stats.missingTickerTriangles++;
            continue;
        }


        TradePrices prices;


        prices.buy1 =
            triangle.buy1;

        prices.buy2 =
            triangle.buy2;

        prices.buy3 =
            triangle.buy3;


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


        stats.validTriangles++;
        stats.analyzedTriangles++;


        auto result =
            calculator.CalculateRealistic(
                settings.startBalance,
                prices,
                settings.commissionPercent / 100.0,
                settings.slippagePercent / 100.0);


        if (stats.analyzedTriangles == 1 ||
            result.profitPercent >
            stats.bestProfitPercent)
        {
            stats.bestProfitPercent =
                result.profitPercent;

            stats.bestTriangle.assetA =
                triangle.assetA;

            stats.bestTriangle.assetB =
                triangle.assetB;

            stats.bestTriangle.assetC =
                triangle.assetC;

            stats.bestTriangle.pairAB.symbol =
                symbolIndex.GetName(triangle.pairAB);

            stats.bestTriangle.pairBC.symbol =
                symbolIndex.GetName(triangle.pairBC);

            stats.bestTriangle.pairCA.symbol =
                symbolIndex.GetName(triangle.pairCA);

            stats.bestRoute =
                symbolIndex.GetName(
                    triangle.pairAB)
                + " -> "
                + symbolIndex.GetName(
                    triangle.pairBC)
                + " -> "
                + symbolIndex.GetName(
                    triangle.pairCA);

            stats.bestPrices =
                prices;

            stats.bestResult =
                result;

            stats.bestTriangle.assetA =
                triangle.assetA;

            stats.bestTriangle.assetB =
                triangle.assetB;

            stats.bestTriangle.assetC =
                triangle.assetC;
        }


        if (result.profitPercent >=
            settings.minProfitPercent)
        {
            stats.profitableTriangles++;
        }
        else
        {
            stats.rejectedTriangles++;
        }
    }

    std::cout
        << "Dirty analyzed this cycle: "
        << dirtyProcessed
        << std::endl;

    return stats;
}