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
    ProfitResult Calculate(
        double startAmount,
        double rate1,
        double rate2,
        double rate3);

    ProfitResult CalculateSpreadAware(
        double startAmount,
        const TradePrices& prices);

    ProfitResult CalculateSpreadAndCommission(
        double startAmount,
        const TradePrices& prices,
        double commissionRate);

    ProfitResult CalculateRealistic(
        double startAmount,
        const TradePrices& prices,
        double commissionRate,
        double slippageRate);
};