#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "IndexedMarketData.h"

class IndexedMarketCache
{
public:

    void Resize(
        std::size_t symbols);

    bool Update(
        std::size_t symbolId,
        double bid,
        double ask,
        std::int64_t updatedAtNs);

    bool Invalidate(
        std::size_t symbolId);

    void ReadTriangle(
        std::size_t symbol1,
        std::size_t symbol2,
        std::size_t symbol3,
        IndexedTicker& ticker1,
        IndexedTicker& ticker2,
        IndexedTicker& ticker3) const;

    IndexedMarketData Snapshot() const;

private:

    struct AtomicTicker
    {
        std::atomic<std::uint64_t>
            version{ 0 };

        std::atomic<double>
            bidPrice{ 0.0 };

        std::atomic<double>
            askPrice{ 0.0 };

        std::atomic<std::int64_t>
            updatedAtNs{ 0 };

        std::atomic<bool>
            initialized{ false };

        std::atomic<bool>
            valid{ false };
    };

    bool ReadTicker(
        std::size_t symbolId,
        IndexedTicker& result) const;

    std::unique_ptr<AtomicTicker[]>
        tickers;

    std::size_t symbolCount = 0;
};
