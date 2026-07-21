#include "IndexedMarketCache.h"

#include <chrono>

void IndexedMarketCache::Resize(
    std::size_t symbols)
{
    tickers =
        std::make_unique<
        AtomicTicker[]>(
            symbols);

    symbolCount =
        symbols;
}

bool IndexedMarketCache::Update(
    std::size_t symbolId,
    double bid,
    double ask,
    std::int64_t updatedAtNs)
{
    if (!tickers ||
        symbolId >= symbolCount)
    {
        return false;
    }

    auto& ticker =
        tickers[symbolId];

    while (true)
    {
        const std::uint64_t versionBefore =
            ticker.version.load(
                std::memory_order_acquire);

        if ((versionBefore & 1ULL) != 0)
        {
            continue;
        }

        const double currentBid =
            ticker.bidPrice.load(
                std::memory_order_relaxed);

        const double currentAsk =
            ticker.askPrice.load(
                std::memory_order_relaxed);

        const bool initialized =
            ticker.initialized.load(
                std::memory_order_relaxed);

        const bool valid =
            ticker.valid.load(
                std::memory_order_relaxed);

        const std::uint64_t versionAfter =
            ticker.version.load(
                std::memory_order_acquire);

        if (versionBefore != versionAfter)
        {
            continue;
        }

        if (initialized &&
            valid &&
            currentBid == bid &&
            currentAsk == ask)
        {
            return false;
        }

        break;
    }

    ticker.version.fetch_add(
        1,
        std::memory_order_acq_rel);

    ticker.bidPrice.store(
        bid,
        std::memory_order_relaxed);

    ticker.askPrice.store(
        ask,
        std::memory_order_relaxed);

    ticker.updatedAtNs.store(
        updatedAtNs,
        std::memory_order_relaxed);

    ticker.initialized.store(
        true,
        std::memory_order_relaxed);

    ticker.valid.store(
        true,
        std::memory_order_relaxed);

    ticker.version.fetch_add(
        1,
        std::memory_order_release);

    return true;
}

bool IndexedMarketCache::Invalidate(
    std::size_t symbolId)
{
    if (!tickers ||
        symbolId >= symbolCount)
    {
        return false;
    }

    auto& ticker =
        tickers[symbolId];

    while (true)
    {
        const std::uint64_t versionBefore =
            ticker.version.load(
                std::memory_order_acquire);

        if ((versionBefore & 1ULL) != 0)
        {
            continue;
        }

        const bool initialized =
            ticker.initialized.load(
                std::memory_order_relaxed);

        const bool valid =
            ticker.valid.load(
                std::memory_order_relaxed);

        const std::uint64_t versionAfter =
            ticker.version.load(
                std::memory_order_acquire);

        if (versionBefore != versionAfter)
        {
            continue;
        }

        if (!initialized ||
            !valid)
        {
            return false;
        }

        break;
    }

    ticker.version.fetch_add(
        1,
        std::memory_order_acq_rel);

    ticker.valid.store(
        false,
        std::memory_order_relaxed);

    ticker.version.fetch_add(
        1,
        std::memory_order_release);

    return true;
}

bool IndexedMarketCache::ReadTicker(
    std::size_t symbolId,
    IndexedTicker& result) const
{
    result =
        IndexedTicker{};

    if (!tickers ||
        symbolId >= symbolCount)
    {
        return false;
    }

    const auto& ticker =
        tickers[symbolId];

    while (true)
    {
        const std::uint64_t versionBefore =
            ticker.version.load(
                std::memory_order_acquire);

        if ((versionBefore & 1ULL) != 0)
        {
            continue;
        }

        const double bid =
            ticker.bidPrice.load(
                std::memory_order_relaxed);

        const double ask =
            ticker.askPrice.load(
                std::memory_order_relaxed);

        const std::int64_t updatedAtNs =
            ticker.updatedAtNs.load(
                std::memory_order_relaxed);

        const bool initialized =
            ticker.initialized.load(
                std::memory_order_relaxed);

        const bool valid =
            ticker.valid.load(
                std::memory_order_relaxed);

        const std::uint64_t versionAfter =
            ticker.version.load(
                std::memory_order_acquire);

        if (versionBefore != versionAfter)
        {
            continue;
        }

        result.bidPrice =
            bid;

        result.askPrice =
            ask;

        result.updatedAt =
            std::chrono::steady_clock::time_point(
                std::chrono::duration_cast<
                std::chrono::steady_clock::duration>(
                    std::chrono::nanoseconds(
                        updatedAtNs)));

        result.version =
            versionAfter;

        result.initialized =
            initialized;

        result.valid =
            valid;

        return true;
    }
}

void IndexedMarketCache::ReadTriangle(
    std::size_t symbol1,
    std::size_t symbol2,
    std::size_t symbol3,
    IndexedTicker& ticker1,
    IndexedTicker& ticker2,
    IndexedTicker& ticker3) const
{
    ReadTicker(
        symbol1,
        ticker1);

    ReadTicker(
        symbol2,
        ticker2);

    ReadTicker(
        symbol3,
        ticker3);
}

IndexedMarketData
IndexedMarketCache::Snapshot() const
{
    IndexedMarketData snapshot;

    snapshot.Resize(
        symbolCount);

    for (std::size_t symbolId = 0;
        symbolId < symbolCount;
        ++symbolId)
    {
        ReadTicker(
            symbolId,
            snapshot.Get(
                symbolId));
    }

    return snapshot;
}
