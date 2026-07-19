#pragma once

#include <string>

#include "../triangle/TriangleBuilder.h"
#include "../profit/TradePrices.h"
#include "../profit/ProfitCalculator.h"

struct Statistics
{
    int totalTriangles = 0;

    int analyzedTriangles = 0;

    int missingTickerTriangles = 0;

    int staleTriangles = 0;

    int validTriangles = 0;

    int profitableTriangles = 0;

    int rejectedTriangles = 0;

    double bestProfitPercent = 0.0;

    std::string bestRoute;

    Triangle bestTriangle;

    TradePrices bestPrices;

    ProfitResult bestResult;
};