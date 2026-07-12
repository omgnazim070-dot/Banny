#pragma once

#include "TradePrices.h"

struct ProfitResult
{
    double startAmount;
    double endAmount;
    double profitPercent;
};

class ProfitCalculator
{
public:
    ProfitResult CalculateRealistic(
        double startAmount,
        const TradePrices& prices,
        double commissionRate,
        double slippageRate);
};