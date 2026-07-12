#pragma once

#include <vector>
#include <string>

class BinanceExchangeInfoClient
{
public:
    std::vector<std::string> GetSymbols();
};