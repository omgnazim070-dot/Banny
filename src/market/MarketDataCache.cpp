#include "MarketDataCache.h"

#include <chrono>

void MarketDataCache::Update(
    const Ticker& ticker)
{
    std::lock_guard<std::mutex> lock(
        mutex);

    Ticker updatedTicker = ticker;

    updatedTicker.lastUpdate =
        std::chrono::steady_clock::now();

    marketData.tickers[
        updatedTicker.symbol] = updatedTicker;
}

MarketData MarketDataCache::GetSnapshot()
{
    std::lock_guard<std::mutex> lock(
        mutex);

    return marketData;
}