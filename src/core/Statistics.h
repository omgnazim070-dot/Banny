#pragma once

#include <string>

struct Statistics
{
    int totalTriangles = 0;

    int analyzedTriangles = 0;

    int missingTickerTriangles = 0;

    int profitableTriangles = 0;

    int rejectedTriangles = 0;

    double bestProfitPercent = 0.0;

    std::string bestRoute;
};