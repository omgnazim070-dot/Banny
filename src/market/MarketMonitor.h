#pragma once

#include "../triangle/TriangleBuilder.h"
#include "../scanner/MarketData.h"
#include "../config/TradingSettings.h"
#include "../core/Statistics.h"

class MarketMonitor
{
public:
    Statistics Run(
        const std::vector<Triangle>& triangles,
        const MarketData& marketData,
        const TradingSettings& settings);
};