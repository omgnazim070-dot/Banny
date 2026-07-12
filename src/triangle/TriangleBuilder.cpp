#include "TriangleBuilder.h"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <iostream>

bool TriangleBuilder::PairExists(
    const std::vector<TradingPair>& pairs,
    const std::string& baseAsset,
    const std::string& quoteAsset)
{
    for (const auto& pair : pairs)
    {
        if (pair.baseAsset == baseAsset &&
            pair.quoteAsset == quoteAsset)
        {
            return true;
        }
    }

    return false;
}

std::vector<Triangle> TriangleBuilder::Build(
    const std::vector<TradingPair>& pairs)
{
    std::vector<Triangle> triangles;

    std::cout
        << "TriangleBuilder started"
        << std::endl;

    std::unordered_map<
        std::string,
        std::unordered_set<std::string>> graph;

    for (const auto& pair : pairs)
    {
        graph[pair.baseAsset]
            .insert(pair.quoteAsset);

        graph[pair.quoteAsset]
            .insert(pair.baseAsset);
    }

    std::set<std::string> uniqueTriangles;

    for (const auto& [assetA, neighborsA] : graph)
    {
        for (const auto& assetB : neighborsA)
        {
            auto itB =
                graph.find(assetB);

            if (itB == graph.end())
            {
                continue;
            }

            for (const auto& assetC : itB->second)
            {
                if (assetC == assetA)
                {
                    continue;
                }

                auto itC =
                    graph.find(assetC);

                if (itC == graph.end())
                {
                    continue;
                }

                if (!itC->second.contains(assetA))
                {
                    continue;
                }

                std::string key =
                    assetA + "|" +
                    assetB + "|" +
                    assetC;

                if (uniqueTriangles.contains(key))
                {
                    continue;
                }

                Triangle triangle;

                triangle.assetA = assetA;
                triangle.assetB = assetB;
                triangle.assetC = assetC;

                triangles.push_back(
                    triangle);

                uniqueTriangles.insert(
                    key);
            }
        }
    }

    std::cout
        << "Triangles found: "
        << triangles.size()
        << std::endl;

    return triangles;
}