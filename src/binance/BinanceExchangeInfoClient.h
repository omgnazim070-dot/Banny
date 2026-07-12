#pragma once

#include <vector>

#include "../market/TradingPair.h"

class BinanceExchangeInfoClient
{
public:
    std::vector<TradingPair> GetSymbols();
};