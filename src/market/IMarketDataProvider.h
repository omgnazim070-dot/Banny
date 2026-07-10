#pragma once

#include "../scanner/MarketData.h"

class IMarketDataProvider
{
public:
    virtual ~IMarketDataProvider() = default;

    virtual MarketData GetMarketData() = 0;
};