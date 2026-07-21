#include "MarketMonitor.h"

#include <chrono>
#include <cstdint>

#include "../arbitrage/ArbitrageEngine.h"
#include "../profit/ProfitCalculator.h"
#include "../profit/TradePrices.h"

namespace
{
    std::int64_t ToNanoseconds(
        std::chrono::steady_clock::time_point time)
    {
        return std::chrono::duration_cast<
            std::chrono::nanoseconds>(
                time.time_since_epoch())
            .count();
    }

    std::uint64_t ClampDurationUs(
        std::int64_t durationNs)
    {
        if (durationNs <= 0)
        {
            return 0;
        }

        return static_cast<std::uint64_t>(
            durationNs / 1000);
    }
}

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

void MarketMonitor::RunBatch(
    const std::vector<IndexedTriangle>& indexedTriangles,
    const std::vector<DirtyTask>& tasks,
    DirtyQueue& queue,
    IndexedMarketCache& marketCache,
    std::vector<TriangleRuntimeState>& runtimeStates,
    const TradingSettings& settings,
    WorkerIntervalStats& stats)
{
    ProfitCalculator calculator;

    const double commissionRate =
        settings.commissionPercent / 100.0;

    const double slippageRate =
        settings.slippagePercent / 100.0;

    const auto analyzeTask =
        [&](const DirtyTask& task)
        {
            const auto calculationStart =
                std::chrono::steady_clock::now();

            const std::int64_t calculationStartNs =
                ToNanoseconds(
                    calculationStart);

            stats.processedTasks++;

            stats.queueWait.Record(
                ClampDurationUs(
                    calculationStartNs -
                    task.queuedAtNs));

            const auto finishTask =
                [&](TriangleRuntimeState* runtimeState)
                {
                    const auto calculationEnd =
                        std::chrono::steady_clock::now();

                    const std::int64_t calculationEndNs =
                        ToNanoseconds(
                            calculationEnd);

                    if (runtimeState != nullptr)
                    {
                        runtimeState->lastCalculationNs =
                            calculationEndNs;
                    }

                    stats.calculation.Record(
                        ClampDurationUs(
                            calculationEndNs -
                            calculationStartNs));

                    stats.totalLatency.Record(
                        ClampDurationUs(
                            calculationEndNs -
                            task.eventTimeNs));
                };

            const TriangleId id =
                task.triangle;

            if (id >= indexedTriangles.size() ||
                id >= runtimeStates.size())
            {
                finishTask(nullptr);
                return;
            }

            const auto& triangle =
                indexedTriangles[id];

            TriangleRuntimeState& runtimeState =
                runtimeStates[id];

            IndexedTicker ticker1;
            IndexedTicker ticker2;
            IndexedTicker ticker3;

            marketCache.ReadTriangle(
                triangle.pairAB,
                triangle.pairBC,
                triangle.pairCA,
                ticker1,
                ticker2,
                ticker3);

            if (runtimeState.hasVersions &&
                runtimeState.version1 == ticker1.version &&
                runtimeState.version2 == ticker2.version &&
                runtimeState.version3 == ticker3.version)
            {
                stats.duplicateVersionSkips++;
                finishTask(&runtimeState);
                return;
            }

            runtimeState.version1 =
                ticker1.version;

            runtimeState.version2 =
                ticker2.version;

            runtimeState.version3 =
                ticker3.version;

            runtimeState.hasVersions =
                true;

            if (!ticker1.initialized ||
                !ticker2.initialized ||
                !ticker3.initialized)
            {
                stats.missingTickerTriangles++;
                finishTask(&runtimeState);
                return;
            }

            if (!ticker1.valid ||
                !ticker2.valid ||
                !ticker3.valid)
            {
                stats.staleTriangles++;
                finishTask(&runtimeState);
                return;
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

            if (prices.price1 <= 0.0 ||
                prices.price2 <= 0.0 ||
                prices.price3 <= 0.0)
            {
                stats.missingTickerTriangles++;
                finishTask(&runtimeState);
                return;
            }

            stats.analyzedTriangles++;

            const auto result =
                calculator.CalculateRealistic(
                    settings.startBalance,
                    prices,
                    commissionRate,
                    slippageRate);

            runtimeState.lastProfitPercent =
                result.profitPercent;

            if (!stats.hasBest ||
                result.profitPercent >
                stats.bestProfitPercent)
            {
                stats.hasBest = true;

                stats.bestProfitPercent =
                    result.profitPercent;

                stats.bestTriangleId =
                    id;
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

            finishTask(&runtimeState);
        };

    for (const DirtyTask& task :
        tasks)
    {
        analyzeTask(
            task);

        DirtyTask recheckTask;

        if (!queue.FinishProcessing(
            task.triangle,
            true,
            recheckTask))
        {
            continue;
        }

        analyzeTask(
            recheckTask);

        DirtyTask unusedTask;

        queue.FinishProcessing(
            task.triangle,
            false,
            unusedTask);
    }
}
