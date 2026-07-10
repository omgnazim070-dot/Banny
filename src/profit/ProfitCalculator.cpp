#include "ProfitCalculator.h"

ProfitResult ProfitCalculator::Calculate(
    double startAmount,
    double rate1,
    double rate2,
    double rate3)
{
    ProfitResult result;

    result.startAmount = startAmount;

    double step1 = startAmount / rate1;
    double step2 = step1 / rate2;
    double finalAmount = step2 * rate3;

    result.endAmount = finalAmount;

    result.profitPercent =
        ((finalAmount - startAmount)
            / startAmount) * 100.0;

    return result;
}