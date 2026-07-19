#pragma once

#include <mutex>

#include "IndexedMarketData.h"

class IndexedMarketCache
{
public:

    void Resize(
        size_t symbols);

    void Update(
        size_t symbolId,
        double bid,
        double ask);

    IndexedMarketData Snapshot();

private:

    IndexedMarketData data;

    std::mutex mutex;
};