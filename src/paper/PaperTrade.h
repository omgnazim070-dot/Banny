#pragma once

#include <string>

struct PaperTrade
{
    std::string route;

    double startBalance;
    double endBalance;

    double profitPercent;
    double profitUsdt;
};