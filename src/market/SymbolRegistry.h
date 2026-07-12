#pragma once

#include <vector>

#include "TradingPair.h"

struct SymbolRegistry
{
    std::vector<TradingPair> pairs;
};