#include "MarketDataCache.h"

void MarketDataCache::Update(
    const Ticker& ticker)
{
    std::lock_guard<std::mutex> lock(
        mutex);

    marketData.tickers[
        ticker.symbol] = ticker;
}

MarketData MarketDataCache::GetSnapshot()
{
    std::lock_guard<std::mutex> lock(
        mutex);

    return marketData;
}