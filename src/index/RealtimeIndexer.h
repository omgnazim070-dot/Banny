#pragma once

#include "SymbolRegistryIndex.h"
#include "TriangleIndex.h"
#include "DirtyQueue.h"
#include "IndexedMarketCache.h"

class RealtimeIndexer
{
public:

    RealtimeIndexer(
        SymbolRegistryIndex& symbols,
        TriangleIndex& triangles,
        DirtyQueue& dirty,
        IndexedMarketCache& market);

    void OnTicker(
        const std::string& symbol,
        double bid,
        double ask);

private:

    SymbolRegistryIndex& symbolIndex;

    TriangleIndex& triangleIndex;

    DirtyQueue& dirtyQueue;

    IndexedMarketCache& marketCache;
};