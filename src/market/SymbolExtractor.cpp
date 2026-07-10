#include "SymbolExtractor.h"

std::vector<std::string>
SymbolExtractor::Extract(
    const MarketData& marketData)
{
    std::vector<std::string> symbols;

    for (const auto& pair :
        marketData.tickers)
    {
        symbols.push_back(
            pair.first);
    }

    return symbols;
}