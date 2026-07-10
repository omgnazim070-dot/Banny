#pragma once

#include "../scanner/MarketData.h"

class MarketSnapshotLogger
{
public:
    bool Save(
        const MarketData& marketData);
};