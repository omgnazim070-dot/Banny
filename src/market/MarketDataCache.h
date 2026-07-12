#pragma once

#include <mutex>

#include "../scanner/MarketData.h"

class MarketDataCache
{
public:

    void Update(
        const Ticker& ticker);

    MarketData GetSnapshot();

private:

    MarketData marketData;

    std::mutex mutex;
};