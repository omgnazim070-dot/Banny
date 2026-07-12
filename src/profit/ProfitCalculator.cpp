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

ProfitResult ProfitCalculator::CalculateSpreadAware(
    double startAmount,
    const TradePrices& prices)
{
    ProfitResult result;

    result.startAmount = startAmount;

    double asset1 =
        startAmount /
        prices.buyPrice1;

    double asset2 =
        asset1 /
        prices.buyPrice2;

    double finalAmount =
        asset2 *
        prices.sellPrice3;

    result.endAmount = finalAmount;

    result.profitPercent =
        ((finalAmount - startAmount)
            / startAmount) * 100.0;

    return result;
}

ProfitResult ProfitCalculator::CalculateSpreadAndCommission(
    double startAmount,
    const TradePrices& prices,
    double commissionRate)
{
    ProfitResult result;

    result.startAmount = startAmount;

    double asset1 =
        startAmount /
        prices.buyPrice1;

    asset1 *= (1.0 - commissionRate);

    double asset2 =
        asset1 /
        prices.buyPrice2;

    asset2 *= (1.0 - commissionRate);

    double finalAmount =
        asset2 *
        prices.sellPrice3;

    finalAmount *= (1.0 - commissionRate);

    result.endAmount = finalAmount;

    result.profitPercent =
        ((finalAmount - startAmount)
            / startAmount) * 100.0;

    return result;
}

ProfitResult ProfitCalculator::CalculateRealistic(
    double startAmount,
    const TradePrices& prices,
    double commissionRate,
    double slippageRate)
{
    ProfitResult result;

    result.startAmount = startAmount;

    if (prices.buyPrice1 <= 0.0 ||
        prices.buyPrice2 <= 0.0 ||
        prices.sellPrice3 <= 0.0)
    {
        result.endAmount = startAmount;
        result.profitPercent = -100.0;
        return result;
    }

    double buyPrice1 =
        prices.buyPrice1 *
        (1.0 + slippageRate);

    double buyPrice2 =
        prices.buyPrice2 *
        (1.0 + slippageRate);

    double sellPrice3 =
        prices.sellPrice3 *
        (1.0 - slippageRate);

    double asset1 =
        startAmount /
        buyPrice1;

    asset1 *=
        (1.0 - commissionRate);

    double asset2 =
        asset1 /
        buyPrice2;

    asset2 *=
        (1.0 - commissionRate);

    double finalAmount =
        asset2 *
        sellPrice3;

    finalAmount *=
        (1.0 - commissionRate);

    result.endAmount =
        finalAmount;

    result.profitPercent =
        ((finalAmount - startAmount)
            / startAmount)
        * 100.0;

    return result;
}