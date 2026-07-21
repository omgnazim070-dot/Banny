#include "RealtimeIndexer.h"

#include <chrono>
#include <limits>
#include <utility>

namespace
{
    std::int64_t GetNowNs()
    {
        return std::chrono::duration_cast<
            std::chrono::nanoseconds>(
                std::chrono::steady_clock::now()
                .time_since_epoch())
            .count();
    }
}

RealtimeIndexer::RealtimeIndexer(
    SymbolRegistryIndex& symbols,
    TriangleIndex& triangles,
    const std::vector<DirtyQueue*>& queues,
    IndexedMarketCache& market)
    :
    symbolIndex(symbols),
    triangleIndex(triangles),
    dirtyQueues(queues),
    marketCache(market)
{
    BuildQueueBatches();
}

void RealtimeIndexer::BuildQueueBatches()
{
    queueBatchesBySymbol.clear();

    queueBatchesBySymbol.resize(
        symbolIndex.Size());

    if (dirtyQueues.empty())
    {
        return;
    }

    for (std::size_t symbol = 0;
        symbol < symbolIndex.Size();
        ++symbol)
    {
        const auto& affectedTriangles =
            triangleIndex.GetTriangles(
                static_cast<SymbolId>(symbol));

        if (affectedTriangles.empty())
        {
            continue;
        }

        std::vector<
            std::vector<TriangleId>>
            groupedTriangles(
                dirtyQueues.size());

        for (const TriangleId triangle :
            affectedTriangles)
        {
            const std::size_t engineId =
                static_cast<std::size_t>(triangle) %
                dirtyQueues.size();

            groupedTriangles[engineId].push_back(
                triangle);
        }

        auto& batches =
            queueBatchesBySymbol[symbol];

        batches.reserve(
            dirtyQueues.size());

        for (std::size_t engineId = 0;
            engineId < dirtyQueues.size();
            ++engineId)
        {
            if (groupedTriangles[engineId].empty() ||
                dirtyQueues[engineId] == nullptr)
            {
                continue;
            }

            QueueBatch batch;

            batch.queue =
                dirtyQueues[engineId];

            batch.triangles =
                std::move(
                    groupedTriangles[engineId]);

            batches.push_back(
                std::move(batch));
        }
    }
}

void RealtimeIndexer::QueueSymbolTriangles(
    SymbolId symbol,
    std::int64_t eventTimeNs)
{
    if (symbol >=
        queueBatchesBySymbol.size())
    {
        return;
    }

    const auto& batches =
        queueBatchesBySymbol[symbol];

    for (const QueueBatch& batch :
        batches)
    {
        batch.queue->MarkBatch(
            batch.triangles,
            eventTimeNs);
    }
}

TickerUpdateResult RealtimeIndexer::OnTicker(
    std::string_view symbol,
    double bid,
    double ask,
    std::int64_t eventTimeNs)
{
    tickerUpdates.fetch_add(
        1,
        std::memory_order_relaxed);

    const SymbolId id =
        symbolIndex.GetId(
            symbol);

    if (id ==
        std::numeric_limits<SymbolId>::max())
    {
        unknownSymbols.fetch_add(
            1,
            std::memory_order_relaxed);

        return TickerUpdateResult::UnknownSymbol;
    }

    const bool changed =
        marketCache.Update(
            id,
            bid,
            ask,
            eventTimeNs);

    if (!changed)
    {
        unchangedPrices.fetch_add(
            1,
            std::memory_order_relaxed);

        return TickerUpdateResult::Unchanged;
    }

    priceChanges.fetch_add(
        1,
        std::memory_order_relaxed);

    QueueSymbolTriangles(
        id,
        eventTimeNs);

    return TickerUpdateResult::Changed;
}

TickerUpdateResult RealtimeIndexer::OnTicker(
    std::string_view symbol,
    double bid,
    double ask)
{
    return OnTicker(
        symbol,
        bid,
        ask,
        GetNowNs());
}

void RealtimeIndexer::OnDisconnected(
    const std::vector<std::string>& symbols)
{
    disconnectEvents.fetch_add(
        1,
        std::memory_order_relaxed);

    const std::int64_t eventTimeNs =
        GetNowNs();

    for (const auto& symbol :
        symbols)
    {
        const SymbolId id =
            symbolIndex.GetId(
                symbol);

        if (id ==
            std::numeric_limits<SymbolId>::max())
        {
            continue;
        }

        if (!marketCache.Invalidate(
            id))
        {
            continue;
        }

        invalidatedSymbols.fetch_add(
            1,
            std::memory_order_relaxed);

        QueueSymbolTriangles(
            id,
            eventTimeNs);
    }
}

std::uint64_t
RealtimeIndexer::TakeTickerUpdates()
{
    return tickerUpdates.exchange(
        0,
        std::memory_order_relaxed);
}

std::uint64_t
RealtimeIndexer::TakePriceChanges()
{
    return priceChanges.exchange(
        0,
        std::memory_order_relaxed);
}

std::uint64_t
RealtimeIndexer::TakeUnchangedPrices()
{
    return unchangedPrices.exchange(
        0,
        std::memory_order_relaxed);
}

std::uint64_t
RealtimeIndexer::TakeUnknownSymbols()
{
    return unknownSymbols.exchange(
        0,
        std::memory_order_relaxed);
}

std::uint64_t
RealtimeIndexer::TakeDisconnectEvents()
{
    return disconnectEvents.exchange(
        0,
        std::memory_order_relaxed);
}

std::uint64_t
RealtimeIndexer::TakeInvalidatedSymbols()
{
    return invalidatedSymbols.exchange(
        0,
        std::memory_order_relaxed);
}
