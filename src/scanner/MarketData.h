#pragma once

#include <string>
#include <unordered_map>

#include "../market/Ticker.h"

struct MarketData
{
    std::unordered_map<
        std::string,
        Ticker
    > tickers;
};