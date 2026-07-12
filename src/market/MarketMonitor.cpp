#include "MarketMonitor.h"

#include "../arbitrage/ArbitrageEngine.h"

Statistics MarketMonitor::Run(
    const std::vector<Triangle>& triangles,
    const MarketData& marketData,
    const TradingSettings& settings)
{
    ArbitrageEngine engine;

    return engine.Analyze(
        triangles,
        marketData,
        settings);
}