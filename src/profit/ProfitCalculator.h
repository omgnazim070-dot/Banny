#pragma once

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
};