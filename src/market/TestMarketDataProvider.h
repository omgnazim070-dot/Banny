#pragma once

#include "IMarketDataProvider.h"

class TestMarketDataProvider :
    public IMarketDataProvider
{
public:
    MarketData GetMarketData() override;
};