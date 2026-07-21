#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "SymbolRegistryIndex.h"
#include "TriangleIndex.h"
#include "DirtyQueue.h"
#include "IndexedMarketCache.h"

enum class TickerUpdateResult
{
    Changed,
    Unchanged,
    UnknownSymbol
};

class RealtimeIndexer
{
public:

    RealtimeIndexer(
        SymbolRegistryIndex& symbols,
        TriangleIndex& triangles,
        const std::vector<DirtyQueue*>& queues,
        IndexedMarketCache& market);

    TickerUpdateResult OnTicker(
        std::string_view symbol,
        double bid,
        double ask,
        std::int64_t eventTimeNs);

    TickerUpdateResult OnTicker(
        std::string_view symbol,
        double bid,
        double ask);

    void OnDisconnected(
        const std::vector<std::string>& symbols);

    std::uint64_t TakeTickerUpdates();

    std::uint64_t TakePriceChanges();

    std::uint64_t TakeUnchangedPrices();

    std::uint64_t TakeUnknownSymbols();

    std::uint64_t TakeDisconnectEvents();

    std::uint64_t TakeInvalidatedSymbols();

private:

    struct QueueBatch
    {
        DirtyQueue* queue = nullptr;

        std::vector<TriangleId>
            triangles;
    };

    void BuildQueueBatches();

    void QueueSymbolTriangles(
        SymbolId symbol,
        std::int64_t eventTimeNs);

    SymbolRegistryIndex& symbolIndex;

    TriangleIndex& triangleIndex;

    std::vector<DirtyQueue*>
        dirtyQueues;

    std::vector<
        std::vector<QueueBatch>>
        queueBatchesBySymbol;

    IndexedMarketCache& marketCache;

    std::atomic<std::uint64_t>
        tickerUpdates{ 0 };

    std::atomic<std::uint64_t>
        priceChanges{ 0 };

    std::atomic<std::uint64_t>
        unchangedPrices{ 0 };

    std::atomic<std::uint64_t>
        unknownSymbols{ 0 };

    std::atomic<std::uint64_t>
        disconnectEvents{ 0 };

    std::atomic<std::uint64_t>
        invalidatedSymbols{ 0 };
};
