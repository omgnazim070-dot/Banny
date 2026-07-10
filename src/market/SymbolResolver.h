#pragma once

#include <string>
#include <vector>

struct TrianglePairs
{
    std::string pair1;
    std::string pair2;
    std::string pair3;
};

class SymbolResolver
{
public:
    TrianglePairs Resolve(
        const std::string& assetA,
        const std::string& assetB,
        const std::string& assetC);
};