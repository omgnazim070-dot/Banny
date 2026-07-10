#pragma once

#include <vector>
#include <string>

#include "../scanner/MarketData.h"

class SymbolExtractor
{
public:
    std::vector<std::string> Extract(
        const MarketData& marketData);
};