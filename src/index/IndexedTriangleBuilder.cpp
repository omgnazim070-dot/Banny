#include "IndexedTriangleBuilder.h"

std::vector<IndexedTriangle>
IndexedTriangleBuilder::Build(
    const std::vector<Triangle>& triangles,
    SymbolRegistryIndex& registry)
{
    std::vector<IndexedTriangle> result;

    result.reserve(
        triangles.size());

    for (const auto& triangle : triangles)
    {
        IndexedTriangle item;

        item.pairAB =
            registry.Register(
                triangle.pairAB.symbol);

        item.pairBC =
            registry.Register(
                triangle.pairBC.symbol);

        item.pairCA =
            registry.Register(
                triangle.pairCA.symbol);

        item.assetA =
            triangle.assetA;

        item.assetB =
            triangle.assetB;

        item.assetC =
            triangle.assetC;

        item.buy1 =
            (triangle.pairAB.quoteAsset ==
                triangle.assetA);

        item.buy2 =
            (triangle.pairBC.quoteAsset ==
                triangle.assetB);

        item.buy3 =
            (triangle.pairCA.quoteAsset ==
                triangle.assetC);

        result.push_back(
            item);
    }

    return result;
}