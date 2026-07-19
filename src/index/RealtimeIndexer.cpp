#include "RealtimeIndexer.h"

#include <iostream>
#include <limits>
#include <cassert>
#include <chrono>

RealtimeIndexer::RealtimeIndexer(
    SymbolRegistryIndex& symbols,
    TriangleIndex& triangles,
    DirtyQueue& dirty,
    IndexedMarketCache& market)
    :
    symbolIndex(symbols),
    triangleIndex(triangles),
    dirtyQueue(dirty),
    marketCache(market)
{
}

void RealtimeIndexer::OnTicker(
    const std::string& symbol,
    double bid,
    double ask)
{
    SymbolId id =
        symbolIndex.GetId(symbol);

    if (id == std::numeric_limits<SymbolId>::max())
    {
        static int unknown = 0;

        unknown++;

        if (unknown <= 10)
        {
            std::cout
                << "UNKNOWN SYMBOL: "
                << symbol
                << std::endl;
        }

        return;
    }

    marketCache.Update(
        id,
        bid,
        ask);

    static int updates = 0;

    ++updates;

    static auto last =
        std::chrono::steady_clock::now();

    auto now =
        std::chrono::steady_clock::now();

    auto elapsed =
        std::chrono::duration_cast<
        std::chrono::milliseconds>(
            now - last);


    if (elapsed.count() < 5)
    {
        return;
    }

    last = now;

    if (updates % 10000 == 0)
    {
        std::cout
            << "RealtimeIndexer updates: "
            << updates
            << std::endl;
    }

    const auto& affected =
        triangleIndex.GetTriangles(id);

    for (TriangleId triangle : affected)
    {
        dirtyQueue.Mark(
            triangle);
    }
}