#pragma once

#include <cstdint>
#include <limits>

#include "LatencyHistogram.h"
#include "../index/SymbolId.h"

struct WorkerIntervalStats
{
    std::uint64_t processedTasks = 0;

    std::uint64_t analyzedTriangles = 0;

    std::uint64_t missingTickerTriangles = 0;

    std::uint64_t staleTriangles = 0;

    std::uint64_t duplicateVersionSkips = 0;

    std::uint64_t profitableTriangles = 0;

    std::uint64_t rejectedTriangles = 0;

    std::uint64_t batches = 0;

    std::uint64_t engineWorkUs = 0;

    std::uint64_t spinWaitUs = 0;

    LatencyHistogram queueWait;

    LatencyHistogram calculation;

    LatencyHistogram totalLatency;

    bool hasBest = false;

    double bestProfitPercent = 0.0;

    TriangleId bestTriangleId =
        std::numeric_limits<TriangleId>::max();

    void Merge(
        const WorkerIntervalStats& other)
    {
        processedTasks +=
            other.processedTasks;

        analyzedTriangles +=
            other.analyzedTriangles;

        missingTickerTriangles +=
            other.missingTickerTriangles;

        staleTriangles +=
            other.staleTriangles;

        duplicateVersionSkips +=
            other.duplicateVersionSkips;

        profitableTriangles +=
            other.profitableTriangles;

        rejectedTriangles +=
            other.rejectedTriangles;

        batches +=
            other.batches;

        engineWorkUs +=
            other.engineWorkUs;

        spinWaitUs +=
            other.spinWaitUs;

        queueWait.Merge(
            other.queueWait);

        calculation.Merge(
            other.calculation);

        totalLatency.Merge(
            other.totalLatency);

        if (other.hasBest &&
            (!hasBest ||
                other.bestProfitPercent >
                bestProfitPercent))
        {
            hasBest = true;

            bestProfitPercent =
                other.bestProfitPercent;

            bestTriangleId =
                other.bestTriangleId;
        }
    }

    void Clear()
    {
        processedTasks = 0;
        analyzedTriangles = 0;
        missingTickerTriangles = 0;
        staleTriangles = 0;
        duplicateVersionSkips = 0;
        profitableTriangles = 0;
        rejectedTriangles = 0;
        batches = 0;
        engineWorkUs = 0;
        spinWaitUs = 0;

        queueWait.Clear();
        calculation.Clear();
        totalLatency.Clear();

        hasBest = false;
        bestProfitPercent = 0.0;

        bestTriangleId =
            std::numeric_limits<TriangleId>::max();
    }
};
