#include "IndexedMarketCache.h"


void IndexedMarketCache::Resize(
    size_t symbols)
{
    std::lock_guard<std::mutex> lock(mutex);

    data.Resize(symbols);
}


void IndexedMarketCache::Update(
    size_t symbolId,
    double bid,
    double ask)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto& ticker =
        data.Get(symbolId);

    ticker.bidPrice = bid;
    ticker.askPrice = ask;
    ticker.valid = true;
}


IndexedMarketData
IndexedMarketCache::Snapshot()
{
    std::lock_guard<std::mutex> lock(mutex);

    return data;
}