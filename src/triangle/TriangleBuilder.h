#pragma once

#include <string>
#include <vector>

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
        const std::vector<std::string>& symbols);

private:
    bool PairExists(
        const std::vector<std::string>& symbols,
        const std::string& pair);
};