#include "Scanner.h"

#include <iostream>

void Scanner::Scan()
{
    std::cout << "Scanning market..." << std::endl;

    int symbols = 1250;
    int routes = 3500;

    double bestProfit = 0.87;

    std::cout
        << "Found "
        << symbols
        << " symbols"
        << std::endl;

    std::cout
        << "Found "
        << routes
        << " triangular routes"
        << std::endl;

    std::cout
        << "Best opportunity: "
        << bestProfit
        << "%"
        << std::endl;
}

MarketData Scanner::GetTestMarketData()
{
    MarketData data;

    data.prices["BTCUSDT"] = 100000.0;

    data.prices["ETHBTC"] = 0.03;
    data.prices["ETHUSDT"] = 3200.0;

    data.prices["BNBBTC"] = 0.006;
    data.prices["BNBUSDT"] = 620.0;

    return data;
}