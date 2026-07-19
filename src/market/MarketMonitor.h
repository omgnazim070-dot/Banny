#pragma once

#include "../triangle/TriangleBuilder.h"
#include "../scanner/MarketData.h"
#include "../config/TradingSettings.h"
#include "../core/Statistics.h"

#include "../index/IndexedTriangleBuilder.h"
#include "../index/TriangleIndex.h"
#include "../index/DirtyQueue.h"
#include "../index/IndexedMarketCache.h"

class MarketMonitor
{
public:

    Statistics Run(
        const std::vector<Triangle>& triangles,
        const MarketData& marketData,
        const TradingSettings& settings);


    Statistics RunDirty(
        const std::vector<IndexedTriangle>& indexedTriangles,
        DirtyQueue& dirtyQueue,
        IndexedMarketCache& marketCache,
        SymbolRegistryIndex& symbolIndex,
        const TradingSettings& settings);
};