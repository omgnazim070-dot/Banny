#include "ProfitCalculator.h"

ProfitResult ProfitCalculator::CalculateRealistic(
    double startAmount,
    const TradePrices& prices,
    double commissionRate,
    double slippageRate)
{
    ProfitResult result;

    result.startAmount = startAmount;

    if (prices.price1 <= 0.0 ||
        prices.price2 <= 0.0 ||
        prices.price3 <= 0.0)
    {
        result.endAmount = startAmount;
        result.profitPercent = -100.0;
        return result;
    }

    double amount = startAmount;

    double p1 =
        prices.price1 *
        (prices.buy1
            ? (1.0 + slippageRate)
            : (1.0 - slippageRate));

    if (prices.buy1)
        amount /= p1;
    else
        amount *= p1;

    amount *= (1.0 - commissionRate);

    double p2 =
        prices.price2 *
        (prices.buy2
            ? (1.0 + slippageRate)
            : (1.0 - slippageRate));

    if (prices.buy2)
        amount /= p2;
    else
        amount *= p2;

    amount *= (1.0 - commissionRate);

    double p3 =
        prices.price3 *
        (prices.buy3
            ? (1.0 + slippageRate)
            : (1.0 - slippageRate));

    if (prices.buy3)
        amount /= p3;
    else
        amount *= p3;

    amount *= (1.0 - commissionRate);

    result.endAmount =
        amount;

    result.profitPercent =
        ((amount - startAmount)
            / startAmount)
        * 100.0;

    return result;
}