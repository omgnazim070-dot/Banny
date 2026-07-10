#pragma once

#include <vector>

#include "../triangle/TriangleBuilder.h"
#include "../profit/ProfitCalculator.h"
#include "../scanner/MarketData.h"
#include "../market/SymbolResolver.h"
#include "../config/TradingSettings.h"
#include "../core/Statistics.h"

class ArbitrageEngine
{
public:
    Statistics Analyze(
        const std::vector<Triangle>& triangles,
        const MarketData& marketData,
        const TradingSettings& settings);
};