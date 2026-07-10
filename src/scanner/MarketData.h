#pragma once

#include <string>
#include <unordered_map>

struct MarketData
{
    std::unordered_map<std::string, double> prices;
};