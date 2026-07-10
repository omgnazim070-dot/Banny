#include "PaperTradeCsvLogger.h"

#include <fstream>

bool PaperTradeCsvLogger::Append(
    const PaperTrade& trade)
{
    std::ofstream file(
        "logs/paper_trades.csv",
        std::ios::app);

    if (!file.is_open())
    {
        return false;
    }

    file
        << trade.route << ","
        << trade.startBalance << ","
        << trade.endBalance << ","
        << trade.profitUsdt << ","
        << trade.profitPercent
        << std::endl;

    return true;
}