#include "MarketSnapshotLogger.h"

#include <fstream>

bool MarketSnapshotLogger::Save(
    const MarketData& marketData)
{
    std::ofstream file(
        "logs/market_snapshot.csv");

    if (!file.is_open())
    {
        return false;
    }

    file
        << "Symbol,Bid,Ask"
        << std::endl;

    for (const auto& item :
        marketData.tickers)
    {
        file
            << item.second.symbol
            << ","
            << item.second.bidPrice
            << ","
            << item.second.askPrice
            << std::endl;
    }

    return true;
}