#pragma once

#include <vector>

#include "IndexedTicker.h"

class IndexedMarketData
{
public:

    void Resize(
        size_t count)
    {
        tickers.resize(count);
    }

    IndexedTicker& Get(
        size_t id)
    {
        return tickers[id];
    }

    const IndexedTicker& Get(
        size_t id) const
    {
        return tickers[id];
    }

private:

    std::vector<IndexedTicker> tickers;
};