#pragma once

#include <cstddef>
#include <vector>

#include "IndexedTicker.h"

class IndexedMarketData
{
public:

    void Resize(
        std::size_t count)
    {
        tickers.resize(
            count);
    }

    IndexedTicker& Get(
        std::size_t id)
    {
        return tickers[id];
    }

    const IndexedTicker& Get(
        std::size_t id) const
    {
        return tickers[id];
    }

private:

    std::vector<IndexedTicker> tickers;
};