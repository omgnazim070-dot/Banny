#pragma once

#include "IMarketDataProvider.h"

class BinanceMarketDataProvider :
    public IMarketDataProvider
{
public:
    MarketData GetMarketData() override;
};