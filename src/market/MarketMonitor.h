#pragma once

#include <vector>

#include "../config/TradingSettings.h"
#include "../core/Statistics.h"
#include "../core/WorkerIntervalStats.h"
#include "../index/DirtyQueue.h"
#include "../index/IndexedMarketCache.h"
#include "../index/TriangleIndex.h"
#include "../index/TriangleRuntimeState.h"
#include "../scanner/MarketData.h"
#include "../triangle/TriangleBuilder.h"

class MarketMonitor
{
public:

    Statistics Run(
        const std::vector<Triangle>& triangles,
        const MarketData& marketData,
        const TradingSettings& settings);

    void RunBatch(
        const std::vector<IndexedTriangle>& indexedTriangles,
        const std::vector<DirtyTask>& tasks,
        DirtyQueue& queue,
        IndexedMarketCache& marketCache,
        std::vector<TriangleRuntimeState>& runtimeStates,
        const TradingSettings& settings,
        WorkerIntervalStats& stats);
};
