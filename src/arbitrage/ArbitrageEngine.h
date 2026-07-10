#pragma once

#include <vector>

#include "../triangle/TriangleBuilder.h"
#include "../profit/ProfitCalculator.h"
#include "../scanner/MarketData.h"
#include "../market/SymbolResolver.h"

class ArbitrageEngine
{
public:
    void Analyze(
        const std::vector<Triangle>& triangles,
        const MarketData& marketData);
};