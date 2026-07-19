#pragma once

#include <vector>

#include "SymbolId.h"

#include <string>

struct IndexedTriangle
{
    SymbolId pairAB;
    SymbolId pairBC;
    SymbolId pairCA;

    std::string assetA;
    std::string assetB;
    std::string assetC;

    bool buy1 = true;
    bool buy2 = true;
    bool buy3 = false;
};
class TriangleIndex
{
public:

    void Build(
        const std::vector<IndexedTriangle>& triangles,
        size_t symbolCount);

    const std::vector<TriangleId>&
        GetTriangles(
            SymbolId symbol) const;

private:

    std::vector<
        std::vector<TriangleId>> index;
};