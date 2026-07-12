#pragma once

#include <string>
#include <vector>

#include "../market/TradingPair.h"

struct Triangle
{
    std::string assetA;
    std::string assetB;
    std::string assetC;
};

class TriangleBuilder
{
public:
    std::vector<Triangle> Build(
        const std::vector<TradingPair>& pairs);

private:
    bool PairExists(
        const std::vector<TradingPair>& pairs,
        const std::string& baseAsset,
        const std::string& quoteAsset);
};