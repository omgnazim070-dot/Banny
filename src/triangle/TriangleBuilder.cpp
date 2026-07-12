#include "TriangleBuilder.h"

#include <algorithm>

bool TriangleBuilder::PairExists(
    const std::vector<std::string>& symbols,
    const std::string& pair)
{
    return std::find(
        symbols.begin(),
        symbols.end(),
        pair) != symbols.end();
}

std::vector<Triangle> TriangleBuilder::Build(
    const std::vector<std::string>& symbols)
{
    std::vector<Triangle> triangles;

    std::vector<std::string> assets =
    {
        "BTC",
        "ETH",
        "USDT",
        "BNB"
    };

    for (const auto& a : assets)
    {
        for (const auto& b : assets)
        {
            for (const auto& c : assets)
            {
                if (a == b || b == c || a == c)
                {
                    continue;
                }

                std::string pairAB = b + a;
                std::string pairBC = c + b;
                std::string pairCA = c + a;

                if (
                    PairExists(symbols, pairAB) &&
                    PairExists(symbols, pairBC) &&
                    PairExists(symbols, pairCA))
                {
                    Triangle t;

                    t.assetA = a;
                    t.assetB = b;
                    t.assetC = c;

                    triangles.push_back(t);
                }
            }
        }
    }

    return triangles;
}